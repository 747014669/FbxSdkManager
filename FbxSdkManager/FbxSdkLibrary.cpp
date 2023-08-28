#include "FbxSdkLibrary.h"

#ifdef IOS_REF
    #undef  IOS_REF
    #define IOS_REF (*(pManager->GetIOSettings()))
#endif


void FbxSdkLibrary::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if( !pManager )
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        return;
    }
    else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

    //Create an IOSettings object. This object holds all import/export settings.
    FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(ios);

    //Load plugins from the executable directory (optional)
    FbxString lPath = FbxGetApplicationDirectory();
    pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
    if( !pScene )
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
    }
}

void FbxSdkLibrary::DestroySdkObjects(FbxManager* pManager)
{
    if( pManager ) pManager->Destroy();
}

bool FbxSdkLibrary::LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pManager,"");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if( !lImportStatus )
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
	    int lAnimStackCount;
	    FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        lAnimStackCount = lImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for(int i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    bool lStatus = lImporter->Import(pScene);
#pragma region 不考虑有密码的
	// if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
	// {
	// 	char lPassword[1024];
	// 	FBXSDK_printf("Please enter password: ");
	//
	// 	lPassword[0] = '\0';
	//
	// 	FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
	// 		scanf("%s", lPassword);
	// 	FBXSDK_CRT_SECURE_NO_WARNING_END
	//
	// 		FbxString lString(lPassword);
	//
	// 	IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
	// 	IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
	//
	// 	lStatus = lImporter->Import(pScene);
	//
	// 	if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
	// 	{
	// 		FBXSDK_printf("\nPassword is wrong, import aborted.\n");
	// 	}
	// }
#pragma endregion

	if (!lStatus || (lImporter->GetStatus() != FbxStatus::eSuccess))
	{
		FBXSDK_printf("********************************************************************************\n");
		if (lStatus)
		{
			FBXSDK_printf("WARNING:\n");
			FBXSDK_printf("   The importer was able to read the file but with errors.\n");
			FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
		}
		else
		{
			FBXSDK_printf("Importer failed to load the file!\n\n");
		}

		if (lImporter->GetStatus() != FbxStatus::eSuccess)
			FBXSDK_printf("   Last error message: %s\n", lImporter->GetStatus().GetErrorString());

		FbxArray<FbxString*> history;
		lImporter->GetStatus().GetErrorStringHistory(history);
		if (history.GetCount() > 1)
		{
			FBXSDK_printf("   Error history stack:\n");
			for (int i = 0; i < history.GetCount(); i++)
			{
				FBXSDK_printf("      %s\n", history[i]->Buffer());
			}
		}
		FbxArrayDelete<FbxString*>(history);
		FBXSDK_printf("********************************************************************************\n");
	}

    // Destroy the importer.
    lImporter->Destroy();

    return lStatus;
}

void FbxSdkLibrary::GetMetaData(FbxScene* pScene, map<string, string>& MetaData)
{
	FbxDocumentInfo* sceneInfo = pScene->GetSceneInfo();
	if (sceneInfo)
	{
		MetaData.insert(pair<string,string>("Title",sceneInfo->mTitle.Buffer()));
		MetaData.insert(pair<string,string>("Subject",sceneInfo->mSubject.Buffer()));
		MetaData.insert(pair<string,string>("Author",sceneInfo->mAuthor.Buffer()));
		MetaData.insert(pair<string,string>("Keywords",sceneInfo->mKeywords.Buffer()));
		MetaData.insert(pair<string,string>("Revision",sceneInfo->mRevision.Buffer()));
		MetaData.insert(pair<string,string>("Comment",sceneInfo->mComment.Buffer()));
	}
}

 void FbxSdkLibrary::GetMeshControlPoint(const FbxMesh* pMesh,vector<Point3D>& ControlPoints)
{
	
	if(pMesh)
	{
		int i, lControlPointsCount = pMesh->GetControlPointsCount();
		FbxVector4* lControlPoints = pMesh->GetControlPoints();
		

		for (i = 0; i < lControlPointsCount; i++)
		{
			Point3D Point = {lControlPoints[i][0],lControlPoints[i][1],lControlPoints[i][2]};
			ControlPoints.push_back(Point);
		}
	}
}

void FbxSdkLibrary::GetMeshPolygonPoints(FbxMesh* pMesh, vector<Polygon3D>& Polygons)
{
	int i, j, lPolygonCount = pMesh->GetPolygonCount();
	FbxVector4* lControlPoints = pMesh->GetControlPoints(); 
	
	//遍历Polygon
	for (i = 0; i < lPolygonCount; i++)
	{
		
		int lPolygonSize = pMesh->GetPolygonSize(i);
		
		Polygon3D Polygon3D;
		//遍历Polygon
		for (j = 0; j < lPolygonSize; j++)
		{
			const int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
			if (lControlPointIndex < 0)
			{
				  FBXSDK_printf("Coordinates: Invalid index found!");
				continue;
			}
			else
			{
				Polygon3D.push_back({lControlPoints[j][0],lControlPoints[j][1],lControlPoints[j][2]});
				FBXSDK_printf("Polygon:%d,X=%f,Y=%f,Z=%f \n",i,lControlPoints[j][0],lControlPoints[j][1],lControlPoints[j][2]);
			}
		}
		Polygons.push_back(Polygon3D);
	}
}

void FbxSdkLibrary::GetMeshVertexColor(FbxMesh* pMesh, vector<vector<RGBA>>& PolygonColors)
{
	
	int i, j, lPolygonCount = pMesh->GetPolygonCount();
	int vertexId = 0;
	//遍历Polygon
	for (i = 0; i < lPolygonCount; i++)
	{
		int l;
		const int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
		
		if (lControlPointIndex < 0)
			continue;
		vector<RGBA> Colors;
		for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
		{
			FbxColor fbx_color;
			FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor( l);
			switch (leVtxc->GetMappingMode())
			{
			default:
				break;
			case FbxGeometryElement::eByControlPoint:
				switch (leVtxc->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					fbx_color = leVtxc->GetDirectArray().GetAt(lControlPointIndex);
					Colors.push_back({fbx_color.mRed,fbx_color.mGreen,fbx_color.mAlpha});
					PolygonColors.push_back(Colors);
					break;
					
				case FbxGeometryElement::eIndexToDirect:
					fbx_color =    leVtxc->GetDirectArray().GetAt(leVtxc->GetIndexArray().GetAt(lControlPointIndex));
					Colors.push_back({fbx_color.mRed,fbx_color.mGreen,fbx_color.mAlpha});
					PolygonColors.push_back(Colors);
					break;
			default:
				break; // other reference modes not shown here!
				}
			
	
			case FbxGeometryElement::eByPolygonVertex:
				{
					switch (leVtxc->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						fbx_color = leVtxc->GetDirectArray().GetAt(vertexId);
						Colors.push_back({fbx_color.mRed,fbx_color.mGreen,fbx_color.mAlpha});
						PolygonColors.push_back(Colors);
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							int id = leVtxc->GetIndexArray().GetAt(vertexId);
							fbx_color = leVtxc->GetDirectArray().GetAt(id);
							Colors.push_back({fbx_color.mRed,fbx_color.mGreen,fbx_color.mAlpha});
							PolygonColors.push_back(Colors);
						}
						break;
					default:
						break; // other reference modes not shown here!
					}
				}
				break;
	
			case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
			case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
			case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
				break;
			}
		}
		vertexId++;
	}
}
