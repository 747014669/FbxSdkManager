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

void FbxSdkLibrary::GetFbxGeometries(FbxScene* const pScene)
{
	if(!pScene)
	{
		return;
	}
	//Geometry参数
	vector<Point3D> ControlPoints;
	FbxGeometryConverter converter = FbxGeometryConverter(pScene->GetFbxManager());
	for(int i = 0;i<pScene->GetGeometryCount();++i)
	{
		FbxMesh* pMesh = (FbxMesh*)pScene->GetGeometry(i);
		//先三角化
		if(!pMesh->IsTriangleMesh())
		{
			pMesh = (FbxMesh*)converter.Triangulate(pScene->GetGeometry(i), true);
		}
		int j,k, PolygonCount = pMesh->GetPolygonCount();
		//ControlPoints
		GetMeshControlPoint(pMesh,ControlPoints);

		//三角面信息
		vector<int> Triangle(3 * PolygonCount);
		vector<RGBA> Colors(3 * PolygonCount);
		vector<Point2D> UVs(3 * PolygonCount);
		vector<Point3D> Normals(3 * PolygonCount);
		vector<Point3D> Tangents(3 * PolygonCount);
		vector<Point3D> Binormals(3 * PolygonCount);
		vector<int> MaterialIds;
		//获得Polygon的信息
		for (j = 0;j<PolygonCount;++j)
		{
			int MaterialId;
			for(k =0;k<pMesh->GetPolygonSize(j);++k )
			{
				//获得三角面信息,小于0则意味着没找到对应的点
				int ControlPointIndex = pMesh->GetPolygonVertex(j, k);
				if(ControlPointIndex >=0)
				{
					int Index = pMesh->GetPolygonSize(j)*j+k;
					Triangle[Index] = ControlPointIndex;
					GetMeshVertexColor(pMesh,j,ControlPointIndex,Colors[Index]);
					GetMeshUV(pMesh,j,ControlPointIndex,k,UVs[Index]);
					GetMeshNormal(pMesh,j,Normals[Index]);
					GetMeshTangent(pMesh,j,Tangents[Index]);
					GetMeshBinormal(pMesh,j,Binormals[Index]);
				}
			}
			GetPolygonMaterialId(pMesh,j,MaterialId);
			MaterialIds.push_back(MaterialId);
		}
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

void FbxSdkLibrary::GetMeshVertexColor(FbxMesh* pMesh,int PolygonIndex, int ControlPointIndex, RGBA& Color)
{
	for (int l = 0; l < pMesh->GetElementVertexColorCount(); l++)
	{
		FbxColor FbxColor;
		FbxGeometryElementVertexColor* Vtxc = pMesh->GetElementVertexColor( l);
		//判断当前定点对应的模式
		switch (Vtxc->GetMappingMode())
		{
				
		case FbxGeometryElement::eByControlPoint:
			{
				switch (Vtxc->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					{
						FbxColor = Vtxc->GetDirectArray().GetAt(ControlPointIndex);
						break;
					}
				case FbxGeometryElement::eIndexToDirect:
					{
						FbxColor = Vtxc->GetDirectArray().GetAt(Vtxc->GetIndexArray().GetAt(ControlPointIndex));
						break;
					}
				default:
					break; // other reference modes not shown here!
				}
				break;
			}
		case FbxGeometryElement::eByPolygonVertex:
			{
				switch (Vtxc->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					{
						FbxColor = Vtxc->GetDirectArray().GetAt(PolygonIndex);
						break;
					}
				case FbxGeometryElement::eIndexToDirect:
					{
						const int id = Vtxc->GetIndexArray().GetAt(PolygonIndex);
						FbxColor = Vtxc->GetDirectArray().GetAt(id);
						break;
					}
				default: break;
				}
			}
			break;
	
		case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
		case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
		case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
			break;
		}
		Color = {FbxColor.mRed,FbxColor.mGreen,FbxColor.mAlpha};
	}
}

void FbxSdkLibrary::GetMeshUV(FbxMesh* pMesh,int PolygonIndex, int ControlPointIndex,int PositionInPolygon, Point2D& UV)
{
	for (int l = 0; l < pMesh->GetElementUVCount(); ++l)
	{
		FbxGeometryElementUV* ElUV = pMesh->GetElementUV( l);
		FBXSDK_printf("%s uv count is %d",pMesh->GetName(),ElUV);
				
		FbxVector2 FbxUV;
		switch (ElUV->GetMappingMode())
		{
		default:
			break;
		case FbxGeometryElement::eByControlPoint:
			switch (ElUV->GetReferenceMode())
			{
		case FbxGeometryElement::eDirect:
				FbxUV = ElUV->GetDirectArray().GetAt(ControlPointIndex);
				break;
		case FbxGeometryElement::eIndexToDirect:
			{
				int id = ElUV->GetIndexArray().GetAt(ControlPointIndex);
				FbxUV = ElUV->GetDirectArray().GetAt(id);

			}
				break;
		default:
			break; // other reference modes not shown here!
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			{
				int TextureUVIndex = pMesh->GetTextureUVIndex(PolygonIndex, PositionInPolygon);
				switch (ElUV->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
				case FbxGeometryElement::eIndexToDirect:
					{
						FbxUV = ElUV->GetDirectArray().GetAt(TextureUVIndex);
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
		UV = {FbxUV[0],FbxUV[1]};
	}
}

void FbxSdkLibrary::GetMeshNormal(FbxMesh* pMesh,int PolygonIndex, Point3D& Normal)
{
	for( int l = 0; l < pMesh->GetElementNormalCount(); ++l)
	{
		FbxGeometryElementNormal* ENolrmal = pMesh->GetElementNormal( l);
		FbxVector4 FbxNormal;
		if(ENolrmal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			switch (ENolrmal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				FbxNormal = ENolrmal->GetDirectArray().GetAt(PolygonIndex);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ENolrmal->GetIndexArray().GetAt(PolygonIndex);
					FbxNormal = ENolrmal->GetDirectArray().GetAt(id);
					
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			Normal = {FbxNormal[0],FbxNormal[1],FbxNormal[2]};	
		}

	}
}

void FbxSdkLibrary::GetMeshTangent(FbxMesh* pMesh, int PolygonIndex,Point3D& Tangent)
{
	for( int l = 0; l < pMesh->GetElementTangentCount(); ++l)
	{
		FbxGeometryElementTangent* ElTangent = pMesh->GetElementTangent( l);
				
		if(ElTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			FbxVector4 FbxTangent;
			switch (ElTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				FbxTangent = ElTangent->GetDirectArray().GetAt(PolygonIndex);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ElTangent->GetIndexArray().GetAt(PolygonIndex);
					FbxTangent = ElTangent->GetDirectArray().GetAt(id);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			Tangent = {FbxTangent[0],FbxTangent[1],FbxTangent[2]};
		}
	}
}

void FbxSdkLibrary::GetMeshBinormal(FbxMesh* pMesh,int PolygonIndex, Point3D& Binormal)
{
	for(int l = 0; l < pMesh->GetElementBinormalCount(); ++l)
	{
		FbxGeometryElementBinormal* ElBinormal = pMesh->GetElementBinormal( l);
				
		if(ElBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			FbxVector4 FbxBinormal;
			switch (ElBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					FbxBinormal = ElBinormal->GetDirectArray().GetAt(PolygonIndex);
				}
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ElBinormal->GetIndexArray().GetAt(PolygonIndex);
					FbxBinormal = ElBinormal->GetDirectArray().GetAt(id);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			Binormal = {FbxBinormal[0],FbxBinormal[1],FbxBinormal[2]};
		}
	}
}

void FbxSdkLibrary::GetPolygonMaterialId(FbxMesh* pMesh, int PolygonIndex, int& Id)
{
	int l;
    //check whether the material maps with only one mesh
    bool lIsAllSame = true;
    for ( l= 0; l < pMesh->GetElementMaterialCount(); l++)
    {

        FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
		if( lMaterialElement->GetMappingMode() == FbxGeometryElement::eByPolygon) 
		{
			lIsAllSame = false;
			break;
		}
    }

    //For eAllSame mapping type, just out the material and texture mapping info once
    if(lIsAllSame)
    {
        for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
        {
            FbxGeometryElementMaterial* MaterialElement = pMesh->GetElementMaterial( l);
			if( MaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame) 
			{
				Id = MaterialElement->GetIndexArray().GetAt(0);
			}
        }
    }
    //For eByPolygon mapping type, just out the material and texture mapping info once
    else
    {
            for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
            {
                FbxGeometryElementMaterial* MaterialElement = pMesh->GetElementMaterial( l);
				Id = MaterialElement->GetIndexArray().GetAt(PolygonIndex);
            }
        }
    
}
