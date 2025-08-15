#include "TextureCsvReader.h"
#include "OptimizerSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	static FString BuildAuditCsvPath(const UOptimizerSettings* Settings)
	{
		const FString SavedDir = FPaths::ProjectSavedDir();
		const FString SubDir = Settings ? Settings->OutputDirectory : TEXT("Saved/MagicOptimizer");
		const FString FullDir = SavedDir / SubDir / TEXT("Audit");
		return FullDir / TEXT("textures.csv");
	}

	static FString BuildRecommendCsvPath(const UOptimizerSettings* Settings)
	{
		const FString SavedDir = FPaths::ProjectSavedDir();
		const FString SubDir = Settings ? Settings->OutputDirectory : TEXT("Saved/MagicOptimizer");
		const FString FullDir = SavedDir / SubDir;
		return FullDir / TEXT("textures_recommend.csv");
	}

	static FString TrimCell(const FString& In)
	{
		FString S = In;
		S.TrimStartAndEndInline();
		if (S.Len() >= 2 && S.StartsWith(TEXT("\"")) && S.EndsWith(TEXT("\"")))
		{
			S = S.Mid(1, S.Len() - 2);
		}
		return S;
	}
}

namespace TextureCsvReader
{
	bool ReadAuditCsv(const UOptimizerSettings* OptimizerSettings, TArray<FTextureAuditRowPtr>& OutRows)
	{
		OutRows.Empty();
		FString CsvPath = BuildAuditCsvPath(OptimizerSettings);
		if (!FPaths::FileExists(CsvPath))
		{
			// Fallback to default Saved location
			CsvPath = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Audit/textures.csv");
			if (!FPaths::FileExists(CsvPath))
			{
				return false;
			}
		}
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *CsvPath))
		{
			return false;
		}
		int32 StartIndex = (Lines.Num() > 0 && Lines[0].StartsWith(TEXT("path"), ESearchCase::IgnoreCase)) ? 1 : 0;
		for (int32 i = StartIndex; i < Lines.Num(); ++i)
		{
			const FString& Line = Lines[i];
			if (Line.TrimStartAndEnd().IsEmpty()) { continue; }
			TArray<FString> Cells;
			Line.ParseIntoArray(Cells, TEXT(","), /*CullEmpty*/ false);
			if (Cells.Num() >= 1)
			{
				FTextureAuditRowPtr Row = MakeShared<FTextureAuditRow>();
				Row->Path = TrimCell(Cells[0]);
				Row->Width = Cells.Num() > 1 ? FCString::Atoi(*TrimCell(Cells[1])) : 0;
				Row->Height = Cells.Num() > 2 ? FCString::Atoi(*TrimCell(Cells[2])) : 0;
				Row->Format = Cells.Num() > 3 ? TrimCell(Cells[3]) : TEXT("");
				OutRows.Add(Row);
			}
		}
		return true;
	}

	bool ReadRecommendationsCsv(const UOptimizerSettings* OptimizerSettings, TArray<FTextureRecRowPtr>& OutRows)
	{
		OutRows.Empty();
		FString CsvPath = BuildRecommendCsvPath(OptimizerSettings);
		if (!FPaths::FileExists(CsvPath))
		{
			CsvPath = FPaths::ProjectSavedDir() / TEXT("MagicOptimizer/Audit/textures_recommend.csv");
			if (!FPaths::FileExists(CsvPath))
			{
				return false;
			}
		}
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *CsvPath))
		{
			return false;
		}
		int32 StartIndex = (Lines.Num() > 0 && Lines[0].StartsWith(TEXT("path"), ESearchCase::IgnoreCase)) ? 1 : 0;
		for (int32 i = StartIndex; i < Lines.Num(); ++i)
		{
			const FString& Line = Lines[i];
			if (Line.TrimStartAndEnd().IsEmpty()) { continue; }
			TArray<FString> Cells;
			Line.ParseIntoArray(Cells, TEXT(","), /*CullEmpty*/ false);
			if (Cells.Num() >= 1)
			{
				FTextureRecRowPtr Row = MakeShared<FTextureRecRow>();
				Row->Path = TrimCell(Cells[0]);
				Row->Width = Cells.Num() > 1 ? FCString::Atoi(*TrimCell(Cells[1])) : 0;
				Row->Height = Cells.Num() > 2 ? FCString::Atoi(*TrimCell(Cells[2])) : 0;
				Row->Format = Cells.Num() > 3 ? TrimCell(Cells[3]) : TEXT("");
				Row->Issues = Cells.Num() > 4 ? TrimCell(Cells[4]) : TEXT("");
				Row->Recommendations = Cells.Num() > 5 ? TrimCell(Cells[5]) : TEXT("");
				OutRows.Add(Row);
			}
		}
		return true;
	}
}


