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

map<uint64_t, map<uint64_t, FbxSection>> FbxSdkLibrary::GetFbxGeometries(FbxScene* const pScene)
{
	map<uint64_t,map<uint64_t, FbxSection>> Geomteries;
	
	if(!pScene)
	{
		return Geomteries;
	}
	//Geometry参数
	vector<FbxVector4> ControlPoints;
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
		vector<FbxColor> Colors(3 * PolygonCount);
		vector<FbxVector2> UVs(3 * PolygonCount);
		vector<FbxVector4> Normals(3 * PolygonCount);
		vector<FbxVector4> Tangents(3 * PolygonCount);
		vector<FbxVector4> Binormals(3 * PolygonCount);
		vector<uint64_t> MaterialIds;
		//获得Polygon的信息
		for (j = 0;j<PolygonCount;++j)
		{
			
			for(k =0;k<pMesh->GetPolygonSize(j);++k )
			{
				//获得三角面信息,小于0则意味着没找到对应的点
				int ControlPointIndex = pMesh->GetPolygonVertex(j, k);
				if(ControlPointIndex >=0)
				{
					int Index = pMesh->GetPolygonSize(j)*j+k;
					Triangle[Index] = ControlPointIndex;
					GetPolygonVertexColor(pMesh,j,ControlPointIndex,Colors[Index]);
					GetPolygonUV(pMesh,j,ControlPointIndex,k,UVs[Index]);
				}
			}
			uint64_t MaterialId;
			GetPolygonNormal(pMesh,j,Normals[j * pMesh->GetPolygonSize(j)]);
			GetPolygonTangent(pMesh,j,Tangents[j * pMesh->GetPolygonSize(j)]);
			GetPolygonBinormal(pMesh,j,Binormals[j * pMesh->GetPolygonSize(j)]);
			GetPolygonMaterialId(pMesh,j,MaterialId);
			MaterialIds.push_back(MaterialId);
		}
		//根据材质分组
		map<uint64_t,FbxSection> Sections;
		for(int MatId = 0 ; MatId < (int)MaterialIds.size(); ++MatId)
		{
			
			if(Sections.count(MaterialIds[MatId]) == 0)
			{
				FbxSection section;
				Sections.insert(pair<uint64_t,FbxSection>(MaterialIds[MatId],section));
			}

			FbxSection* Geom = &Sections[MaterialIds[MatId]];
			for(int l=0;l<3;++l)
			{
				Geom->Triangle.push_back(Triangle[3*MatId+l]);
				Geom->Colors.push_back(Colors[3*MatId+l]);
				Geom->UVs.push_back(UVs[3*MatId+l]);
				Geom->Normals.push_back(Normals[3*MatId+l]);
				Geom->Tangents.push_back(Tangents[3*MatId+l]);
				Geom->Binormals.push_back(Binormals[3*MatId+l]);
				Geom->MaterialIds = MaterialIds[MatId];
			}
		}

		Geomteries.insert(pair<uint64_t,map<uint64_t, FbxSection>>(pMesh->GetUniqueID(),Sections));
	}
	return Geomteries;
}

void FbxSdkLibrary::GetMeshControlPoint(const FbxMesh* pMesh,vector<FbxVector4>& ControlPoints)
{
	if(pMesh)
	{
		int i, lControlPointsCount = pMesh->GetControlPointsCount();
		FbxVector4* lControlPoints = pMesh->GetControlPoints();
		
		for (i = 0; i < lControlPointsCount; i++)
		{
			ControlPoints.push_back(lControlPoints[i]);
		}
	}
}

void FbxSdkLibrary::GetPolygonVertexColor(FbxMesh* pMesh, int PolygonIndex, int ControlPointIndex, FbxColor& Color)
{
	for (int l = 0; l < pMesh->GetElementVertexColorCount(); l++)
	{
		
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
						Color = Vtxc->GetDirectArray().GetAt(ControlPointIndex);
						break;
					}
				case FbxGeometryElement::eIndexToDirect:
					{
						Color = Vtxc->GetDirectArray().GetAt(Vtxc->GetIndexArray().GetAt(ControlPointIndex));
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
						Color = Vtxc->GetDirectArray().GetAt(PolygonIndex);
						break;
					}
				case FbxGeometryElement::eIndexToDirect:
					{
						const int id = Vtxc->GetIndexArray().GetAt(PolygonIndex);
						Color = Vtxc->GetDirectArray().GetAt(id);
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
		
	}
}

void FbxSdkLibrary::GetPolygonUV(FbxMesh* pMesh, int PolygonIndex, int ControlPointIndex, int PositionInPolygon,FbxVector2& UV)
{
	for (int l = 0; l < pMesh->GetElementUVCount(); ++l)
	{
		FbxGeometryElementUV* ElUV = pMesh->GetElementUV( l);
		
		switch (ElUV->GetMappingMode())
		{
		default:
			break;
		case FbxGeometryElement::eByControlPoint:
			switch (ElUV->GetReferenceMode())
			{
		case FbxGeometryElement::eDirect:
				UV = ElUV->GetDirectArray().GetAt(ControlPointIndex);
				break;
		case FbxGeometryElement::eIndexToDirect:
			{
				int id = ElUV->GetIndexArray().GetAt(ControlPointIndex);
				UV = ElUV->GetDirectArray().GetAt(id);

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
						UV = ElUV->GetDirectArray().GetAt(TextureUVIndex);
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
}

void FbxSdkLibrary::GetPolygonNormal(FbxMesh* pMesh, int PolygonIndex, FbxVector4& Normal)
{
	for( int l = 0; l < pMesh->GetElementNormalCount(); ++l)
	{
		FbxGeometryElementNormal* ENolrmal = pMesh->GetElementNormal( l);
		
		if(ENolrmal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			switch (ENolrmal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				Normal = ENolrmal->GetDirectArray().GetAt(PolygonIndex);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ENolrmal->GetIndexArray().GetAt(PolygonIndex);
					Normal = ENolrmal->GetDirectArray().GetAt(id);
					
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			*(&Normal + 1) = Normal;
			*(&Normal + 2) = Normal;
		}
	}
}

void FbxSdkLibrary::GetPolygonTangent(FbxMesh* pMesh, int PolygonIndex, FbxVector4& Tangent)
{
	for( int l = 0; l < pMesh->GetElementTangentCount(); ++l)
	{
		FbxGeometryElementTangent* ElTangent = pMesh->GetElementTangent( l);
				
		if(ElTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			switch (ElTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				Tangent = ElTangent->GetDirectArray().GetAt(PolygonIndex);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ElTangent->GetIndexArray().GetAt(PolygonIndex);
					Tangent = ElTangent->GetDirectArray().GetAt(id);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			*(&Tangent + 1) = Tangent;
			*(&Tangent + 2) = Tangent;
		}
	}
}

void FbxSdkLibrary::GetPolygonBinormal(FbxMesh* pMesh, int PolygonIndex, FbxVector4& Binormal)
{
	for(int l = 0; l < pMesh->GetElementBinormalCount(); ++l)
	{
		FbxGeometryElementBinormal* ElBinormal = pMesh->GetElementBinormal( l);
				
		if(ElBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			switch (ElBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
				{
					Binormal = ElBinormal->GetDirectArray().GetAt(PolygonIndex);
				}
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ElBinormal->GetIndexArray().GetAt(PolygonIndex);
					Binormal = ElBinormal->GetDirectArray().GetAt(id);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			*(&Binormal + 1) = Binormal;
			*(&Binormal + 2) = Binormal;
		}
	}
}

void FbxSdkLibrary::GetPolygonMaterialId(FbxMesh* pMesh, int PolygonIndex, uint64_t& Id)
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
				FbxSurfaceMaterial* Material = pMesh->GetNode()->GetMaterial(MaterialElement->GetIndexArray().GetAt(0));
				Id = Material->GetUniqueID();
				break;
			}
		}
		//没有材质
		// if(l == 0)
		// {
		// 	Id = 0;
		// }
	}
	//For eByPolygon mapping type, just out the material and texture mapping info once
	else
	{
		for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
		{
			int Index = -1;
			FbxGeometryElementMaterial* MaterialElement = pMesh->GetElementMaterial( l);
			Index = MaterialElement->GetIndexArray().GetAt(PolygonIndex);
			if(Index >= 0)
			{
				FbxSurfaceMaterial* Material = pMesh->GetNode()->GetMaterial(Index);
				Id =  Material->GetUniqueID();
				break;
			}
			
		}
	}

}

static const FbxImplementation* LookForImplementation(FbxSurfaceMaterial* pMaterial)
{
	const FbxImplementation* lImplementation = nullptr;
	if (!lImplementation) lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_CGFX);
	if (!lImplementation) lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_HLSL);
	if (!lImplementation) lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_SFX);
	if (!lImplementation) lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_OGS);
	if (!lImplementation) lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_SSSL);
	return lImplementation;    
}

void FbxSdkLibrary::GetFbxMaterials(FbxScene* pScene, map<uint64_t, FbxMaterialsInfo>& MaterialInfos)
{
	const int MaterialCount = pScene->GetMaterialCount();
	

	FbxPropertyT<FbxDouble> MFbxDouble1;
	for(int i = 0;i<MaterialCount;i++)
	{
		FbxSurfaceMaterial* Material = pScene->GetMaterial(i);

#pragma region 遇到了再说
		//    const FbxImplementation* lImplementation = LookForImplementation(Material);
		// if(lImplementation)
		//        {
		// 	
		//            const FbxBindingTable* RootTable = lImplementation->GetRootTable();
		//            FbxString FileName = RootTable->DescAbsoluteURL.Get();
		//            FbxString TechniqueName = RootTable->DescTAG.Get(); 
		//
		//
		//            const FbxBindingTable* Table = lImplementation->GetRootTable();
		//            size_t lEntryNum = Table->GetEntryCount();
		//
		//            for(int i=0;i <(int)lEntryNum; ++i)
		//            {
		//                const FbxBindingTableEntry& Entry = Table->GetEntry(i);
		//                const char* EntrySrcType = Entry.GetEntryType(true);
		//            	
		//                FbxProperty FbxProp;
		//                if ( strcmp( FbxPropertyEntryView::sEntryType, EntrySrcType ) == 0 )
		//                {   
		//                    FbxProp = Material->FindPropertyHierarchical(Entry.GetSource()); 
		//                    if(!FbxProp.IsValid())
		//                    {
		//                        FbxProp = Material->RootProperty.FindHierarchical(Entry.GetSource());
		//                    }
		//                	
		//                }
		//                else if( strcmp( FbxConstantEntryView::sEntryType, EntrySrcType ) == 0 )
		//                {
		//                    FbxProp = lImplementation->GetConstants().FindHierarchical(Entry.GetSource());
		//                }
		//            	
		//                if(FbxProp.IsValid())
		//                {
		//                    if( FbxProp.GetSrcObjectCount<FbxTexture>() > 0 )
		//                    {
		//                        //do what you want with the textures
		//                        for(int j=0; j<FbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
		//                        {
		//                            FbxFileTexture *lTex = FbxProp.GetSrcObject<FbxFileTexture>(j);
		//                            DisplayString("           File Texture: ", lTex->GetFileName());
		//                        }
		//                        for(int j=0; j<FbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
		//                        {
		//                            FbxLayeredTexture *lTex = FbxProp.GetSrcObject<FbxLayeredTexture>(j);
		//                            DisplayString("        Layered Texture: ", lTex->GetName());
		//                        }
		//                        for(int j=0; j<FbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
		//                        {
		//                            FbxProceduralTexture *lTex = FbxProp.GetSrcObject<FbxProceduralTexture>(j);
		//                            DisplayString("     Procedural Texture: ", lTex->GetName());
		//                        }
		//                    }
		//                    else
		//                    {
		//                        FbxDataType lFbxType = FbxProp.GetPropertyDataType();
		//                        FbxString blah = lFbxType.GetName();
		//                        if(FbxBoolDT == lFbxType)
		//                        {
		//                            DisplayBool("                Bool: ", FbxProp.Get<FbxBool>() );
		//                        }
		//                        else if ( FbxIntDT == lFbxType ||  FbxEnumDT  == lFbxType )
		//                        {
		//                            DisplayInt("                Int: ", FbxProp.Get<FbxInt>());
		//                        }
		//                        else if ( FbxFloatDT == lFbxType)
		//                        {
		//                            DisplayDouble("                Float: ", FbxProp.Get<FbxFloat>());
		//
		//                        }
		//                        else if ( FbxDoubleDT == lFbxType)
		//                        {
		//                            DisplayDouble("                Double: ", FbxProp.Get<FbxDouble>());
		//                        }
		//                        else if ( FbxStringDT == lFbxType
		//                            ||  FbxUrlDT  == lFbxType
		//                            ||  FbxXRefUrlDT  == lFbxType )
		//                        {
		//                            DisplayString("                String: ", FbxProp.Get<FbxString>().Buffer());
		//                        }
		//                        else if ( FbxDouble2DT == lFbxType)
		//                        {
		//                            FbxDouble2 lDouble2 = FbxProp.Get<FbxDouble2>();
		//                            FbxVector2 lVect;
		//                            lVect[0] = lDouble2[0];
		//                            lVect[1] = lDouble2[1];
		//
		//                            Display2DVector("                2D vector: ", lVect);
		//                        }
		//                        else if ( FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
		//                        {
		//                            FbxDouble3 lDouble3 = FbxProp.Get<FbxDouble3>();
		//
		//
		//                            FbxVector4 lVect;
		//                            lVect[0] = lDouble3[0];
		//                            lVect[1] = lDouble3[1];
		//                            lVect[2] = lDouble3[2];
		//                            Display3DVector("                3D vector: ", lVect);
		//                        }
		//
		//                        else if ( FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
		//                        {
		//                            FbxDouble4 lDouble4 = FbxProp.Get<FbxDouble4>();
		//                            FbxVector4 lVect;
		//                            lVect[0] = lDouble4[0];
		//                            lVect[1] = lDouble4[1];
		//                            lVect[2] = lDouble4[2];
		//                            lVect[3] = lDouble4[3];
		//                            Display4DVector("                4D vector: ", lVect);
		//                        }
		//                        else if ( FbxDouble4x4DT == lFbxType)
		//                        {
		//                            FbxDouble4x4 lDouble44 = FbxProp.Get<FbxDouble4x4>();
		//                            for(int j=0; j<4; ++j)
		//                            {
		//
		//                                FbxVector4 lVect;
		//                                lVect[0] = lDouble44[j][0];
		//                                lVect[1] = lDouble44[j][1];
		//                                lVect[2] = lDouble44[j][2];
		//                                lVect[3] = lDouble44[j][3];
		//                                Display4DVector("                4x4D vector: ", lVect);
		//                            }
		//
		//                        }
		//                    }
		//
		//                }   
		//            }
		//        }
#pragma endregion
	
		FbxMaterialsInfo MaterialInfo = FbxMaterialsInfo();
		if (Material->GetClassId().Is(FbxSurfacePhong::ClassId))
		{
			// We found a Phong material.  Display its properties.

			// Display the Ambient Color
	
			MaterialInfo.Ambient.Color = ((FbxSurfacePhong *) Material)->Ambient.Get();
			MaterialInfo.Ambient.Texture = GetMaterialTexture(Material, FbxSurfacePhong::sAmbient);
			
			MaterialInfo.Diffuse.Color = ((FbxSurfacePhong *) Material)->Diffuse.Get();
			MaterialInfo.Diffuse.Texture = GetMaterialTexture(Material, FbxSurfacePhong::sDiffuse);

			// Display the Specular Color (unique to Phong materials)
			MaterialInfo.Specular.Color = ((FbxSurfacePhong *) Material)->Specular.Get();
			MaterialInfo.Specular.Texture = GetMaterialTexture(Material, FbxSurfacePhong::sSpecular);
            	
			// Display the Emissive Color
			MaterialInfo.Emissive.Color = ((FbxSurfacePhong *) Material)->Emissive.Get();
			MaterialInfo.Emissive.Texture = GetMaterialTexture(Material, FbxSurfacePhong::sEmissive);

			//Opacity is Transparency factor now
			MFbxDouble1 =((FbxSurfacePhong *) Material)->TransparencyFactor;
			MaterialInfo.Opacity.Factory = 1.0-MFbxDouble1.Get();

			// Display the Shininess
			MFbxDouble1 =((FbxSurfacePhong *) Material)->Shininess;
			MaterialInfo.Opacity.Factory = MFbxDouble1.Get();
            	
			// Display the Reflectivity
			MFbxDouble1 =((FbxSurfacePhong *) Material)->ReflectionFactor;
			MaterialInfo.Reflectivity.Factory = MFbxDouble1.Get();
		}
		else if(Material->GetClassId().Is(FbxSurfaceLambert::ClassId) )
		{
			// We found a Lambert material. Display its properties.
			// Display the Ambient Color
			MaterialInfo.Ambient.Color = ((FbxSurfaceLambert *)Material)->Ambient.Get();
			MaterialInfo.Ambient.Texture = GetMaterialTexture(Material, FbxSurfaceLambert::sAmbient);

			// Display the Diffuse Color
			MaterialInfo.Diffuse.Color = ((FbxSurfaceLambert *)Material)->Diffuse.Get();
			MaterialInfo.Diffuse.Texture = GetMaterialTexture(Material, FbxSurfaceLambert::sDiffuse);

			// Display the Emissive
			MaterialInfo.Emissive.Color = ((FbxSurfaceLambert *)Material)->Emissive.Get();
			MaterialInfo.Emissive.Texture = GetMaterialTexture(Material, FbxSurfaceLambert::sEmissive);

			// Display the Opacity
			MFbxDouble1 =((FbxSurfaceLambert *)Material)->TransparencyFactor;
			MaterialInfo.Opacity.Factory =  1.0-MFbxDouble1.Get();
			MaterialInfo.Opacity.Texture = GetMaterialTexture(Material, FbxSurfaceLambert::sTransparencyFactor);
		}
		MaterialInfos.insert(pair<uint64_t,FbxMaterialsInfo>(Material->GetUniqueID(),MaterialInfo));
	}
}

const char* FbxSdkLibrary::GetMaterialTexture(FbxSurfaceMaterial* pMaterial,const char* Property)
{
	//先找到Property对应的LayeredTexture
	FbxProperty MProperty = pMaterial->FindProperty(Property);
	int TextureCount = MProperty.GetSrcObjectCount<FbxTexture>();
	if(TextureCount == 0)
	{
		return "";
	}
	
	for(int i=0; i<TextureCount; ++i)
	{
		const char* FileName;
		FbxLayeredTexture *LayeredTexture = MProperty.GetSrcObject<FbxLayeredTexture>(i);
		if(LayeredTexture)
		{
			//获得Texture
			for(int j = 0; j<LayeredTexture->GetSrcObjectCount<FbxTexture>(); ++j)
			{
				FbxTexture* Texture = LayeredTexture->GetSrcObject<FbxTexture>(j);
				if(Texture)
				{
					FileName =  FbxCast<FbxFileTexture>(Texture)->GetFileName();
					return FileName;
				}
			}
		}
		else
		{
			//no layered texture simply get on the property
			FbxTexture* Texture = MProperty.GetSrcObject<FbxTexture>(i);
			FileName = FbxCast<FbxFileTexture>(Texture)->GetFileName();
			return FileName;
		}
	}
	

	return "";
}
