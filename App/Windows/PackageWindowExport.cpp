#include "PackageWindow.h"
#include "ProgressWindow.h"
#include "../App.h"

#include <filesystem>

#include <Utils/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/Cast.h>
#include <Tera/UObject.h>
#include <Tera/UObjectRedirector.h>
#include <Tera/UTexture.h>
#include <Tera/USkeletalMesh.h>
#include <Tera/UStaticMesh.h>
#include <Tera/USoundNode.h>
#include <Tera/USpeedTree.h>

#include <Utils/FbxUtils.h>
#include <Utils/TextureProcessor.h>

void PackageWindow::OnBulkPackageExport(PACKAGE_INDEX objIndex)
{

	static const std::vector<std::string> filter = { UTexture2D::StaticClassName(), USkeletalMesh::StaticClassName(), UStaticMesh::StaticClassName(), USoundNodeWave::StaticClassName(), USpeedTree::StaticClassName() };

	std::function<void(FObjectExport*, std::vector<FObjectExport*>&)> rCollectExports;
	rCollectExports = [&](FObjectExport* exp, std::vector<FObjectExport*>& output) {
		FString className = exp->GetClassName();
		if (!exp)
		{
			return;
		}
		if (className == NAME_Package)
		{
			for (FObjectExport* inner : exp->Inner)
			{
				rCollectExports(inner, output);
			}
			return;
		}

		if (className == UObjectRedirector::StaticClassName())
		{
			try
			{
				if (UObjectRedirector* redirector = Cast<UObjectRedirector>(Package->GetObject(exp)))
				{
					if (UObject* object = redirector->GetObject())
					{
						className = object->GetClassName();
					}
				}
			}
			catch (...)
			{
				return;
			}
		}

		if (std::find(filter.begin(), filter.end(), className.UTF8()) != filter.end())
		{
			output.push_back(exp);
		}
	};
	std::vector<FObjectExport*> exports;
	FObjectExport* rootExport = nullptr;
	if (objIndex == FAKE_EXPORT_ROOT)
	{
		std::vector<FObjectExport*> root = Package->GetRootExports();
		for (FObjectExport* exp : root)
		{
			rCollectExports(exp, exports);
		}
	}
	else
	{
		rootExport = Package->GetExportObject(objIndex);
		rCollectExports(rootExport, exports);
	}

	if (exports.empty())
	{
		wxMessageBox("The package has no supported objects to export.", "Nothing to export!", wxICON_INFORMATION, this);
		return;
	}

	wxDirDialog dlg(NULL, "Select a directory to extract packages to...", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg.ShowModal() != wxID_OK || dlg.GetPath().empty())
	{
		return;
	}

	const std::filesystem::path root = std::filesystem::path(dlg.GetPath().ToStdWstring()) / (rootExport ? rootExport->GetObjectName().WString() : Package->GetPackageName().WString());
	std::vector<FObjectExport*> failedExports;

	ProgressWindow progress(this, wxT("Exporting..."));
	progress.SetMaxProgress(exports.size());

	std::thread([&] {
		for (int idx = 0; idx < exports.size(); ++idx)
		{
			if (progress.IsCanceled())
			{
				SendEvent(&progress, UPDATE_PROGRESS_FINISH);
				return;
			}
			FObjectExport* exp = exports[idx];
			SendEvent(&progress, UPDATE_PROGRESS, idx);
			SendEvent(&progress, UPDATE_PROGRESS_DESC, wxString("Exporting: ") + exp->GetObjectName().WString());
			std::filesystem::path dest(root);
			std::vector<std::wstring> pathComponents;
			FObjectExport* outer = exp->Outer;
			while (outer && outer != rootExport)
			{
				pathComponents.insert(pathComponents.begin(), outer->GetObjectName().WString());
				outer = outer->Outer;
			}
			for (const auto& component : pathComponents)
			{
				dest /= component;
			}

			std::error_code err;
			if (!std::filesystem::exists(dest, err))
			{
				if (!std::filesystem::create_directories(dest, err))
				{
					failedExports.push_back(exp);
					LogE("Failed to create a directory to export %s", exp->GetObjectName().UTF8().c_str());
					continue;
				}
			}

			dest /= exp->GetObjectName().WString();

			UObject* obj = nullptr;
			try
			{
				if (exp->GetClassName() == UObjectRedirector::StaticClassName())
				{
					obj = Cast<UObjectRedirector>(exp->Package->GetObject(exp))->GetObject();
				}
				else
				{
					obj = exp->Package->GetObject(exp);
				}
			}
			catch (...)
			{
				failedExports.push_back(exp);
				LogE("Failed to load %s", exp->GetObjectName().UTF8().c_str());
				continue;
			}

			if (!obj)
			{
				failedExports.push_back(exp);
				LogE("Failed to load %s", exp->GetObjectName().UTF8().c_str());
				continue;
			}

			if (obj->GetClassName() == UTexture2D::StaticClassName())
			{
				UTexture2D* texture = Cast<UTexture2D>(obj);
				if (!texture)
				{
					failedExports.push_back(exp);
					LogE("%s is not a texture", exp->GetObjectName().UTF8().c_str());
					continue;
				}
				FTexture2DMipMap* mip = nullptr;
				for (FTexture2DMipMap* mipmap : texture->Mips)
				{
					if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
					{
						mip = mipmap;
						break;
					}
				}
				if (!mip)
				{
					failedExports.push_back(exp);
					continue;
				}

				dest.replace_extension(HasAVX2() ? "tga" : "dds");

				TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
				TextureProcessor::TCFormat outputFormat = HasAVX2() ? TextureProcessor::TCFormat::TGA : TextureProcessor::TCFormat::DDS;

				if (texture->Format == PF_DXT1)
				{
					inputFormat = TextureProcessor::TCFormat::DXT1;
				}
				else if (texture->Format == PF_DXT3)
				{
					inputFormat = TextureProcessor::TCFormat::DXT3;
				}
				else if (texture->Format == PF_DXT5)
				{
					inputFormat = TextureProcessor::TCFormat::DXT5;
				}
				else if (texture->Format == PF_A8R8G8B8)
				{
					inputFormat = TextureProcessor::TCFormat::ARGB8;
				}
				else if (texture->Format == PF_G8)
				{
					inputFormat = TextureProcessor::TCFormat::G8;
				}
				else
				{
					failedExports.push_back(exp);
					LogE("%s has unsupported pixel format!", exp->GetObjectName().UTF8().c_str());
					continue;
				}

				TextureProcessor processor(inputFormat, outputFormat);

				processor.SetInputData(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
				processor.SetOutputPath(W2A(dest.wstring()));
				processor.SetInputDataDimensions(mip->SizeX, mip->SizeY);

				try
				{
					if (!processor.Process())
					{
						failedExports.push_back(exp);
						LogE("Failed to export %s: %s", exp->GetObjectName().UTF8().c_str(), processor.GetError().c_str());
						continue;
					}
				}
				catch (...)
				{
					failedExports.push_back(exp);
					LogE("Failed to export %s!", exp->GetObjectName().UTF8().c_str());
					continue;
				}
				continue;
			}
			if (obj->GetClassName() == USoundNodeWave::StaticClassName())
			{
				dest.replace_extension("ogg");
				if (USoundNodeWave* wave = Cast<USoundNodeWave>(obj))
				{
					const void* soundData = wave->GetResourceData();
					const int32 soundDataSize = wave->GetResourceSize();
					std::ofstream s(dest, std::ios::out | std::ios::trunc | std::ios::binary);
					s.write((const char*)soundData, soundDataSize);
				}
				else
				{
					failedExports.push_back(exp);
					LogE("%s is not a SoundNodeWave!", exp->GetObjectName().UTF8().c_str());
				}
				continue;
			}
			if (obj->GetClassName() == USpeedTree::StaticClassName())
			{
				dest.replace_extension("spt");
				if (USpeedTree* tree = Cast<USpeedTree>(obj))
				{
					void* sptData = nullptr;
					FILE_OFFSET sptDataSize = 0;
					if (!tree->GetSptData(&sptData, &sptDataSize, false) || !sptDataSize || !sptData)
					{
						failedExports.push_back(exp);
						LogE("Failed to export %s!", exp->GetObjectName().UTF8().c_str());
						continue;
					}
					std::ofstream s(dest, std::ios::out | std::ios::trunc | std::ios::binary);
					s.write((const char*)sptData, sptDataSize);
					free(sptData);
				}
				else
				{
					LogE("F%s is not a SpeedTree!", exp->GetObjectName().UTF8().c_str());
					failedExports.push_back(exp);
				}
				continue;
			}
			if (obj->GetClassName() == UStaticMesh::StaticClassName() || obj->GetClassName() == USkeletalMesh::StaticClassName())
			{
				FbxExportContext ctx;
				ctx.Path = dest.replace_extension("fbx").wstring();

				FbxUtils utils;
				if (exp->GetClassName() == UStaticMesh::StaticClassName())
				{
					if (UStaticMesh* mesh = Cast<UStaticMesh>(obj))
					{
						if (!utils.ExportStaticMesh(mesh, ctx))
						{
							failedExports.push_back(exp);
							LogE("Failed to export %s!", exp->GetObjectName().UTF8().c_str());
						}
					}
					else
					{
						failedExports.push_back(exp);
						LogE("%s is not a StaticMesh!", exp->GetObjectName().UTF8().c_str());
					}
				}
				else
				{
					ctx.ExportSkeleton = true;
					if (USkeletalMesh* mesh = Cast<USkeletalMesh>(obj))
					{
						if (!utils.ExportSkeletalMesh(mesh, ctx))
						{
							failedExports.push_back(exp);
							LogE("Failed to export %s!", exp->GetObjectName().UTF8().c_str());
						}
					}
					else
					{
						failedExports.push_back(exp);
						LogE("%s is not a SkeletalMesh!", exp->GetObjectName().UTF8().c_str());
					}
				}
				continue;
			}
		}
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();

	progress.ShowModal();

	if (failedExports.empty())
	{
		wxMessageBox(wxString::Format("Exported %d objects.", exports.size()), "Done!", wxICON_INFORMATION, this);
	}
	else
	{
		wxString desc = failedExports.size() == exports.size() ? "Failed to export objects!" : "Failed to export some objects!";
		desc += "See the log for details.";
		wxMessageBox(desc, "Warning!", wxICON_WARNING, this);
	}
}