#include "FbxSdkLibrary.h"
#include "FbxSdkException.h"

using std::vector;
using std::map;
using std::pair;

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
        FbxErrorHandler::LogError("Unable to create FBX Manager!");
        throw FbxSdkException(FbxSdkException::MANAGER_CREATE_FAILED, "Failed to create FbxManager instance");
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
        FbxErrorHandler::LogError("Unable to create FBX scene!");
        throw FbxSdkException(FbxSdkException::SCENE_CREATE_FAILED, "Failed to create FbxScene instance");
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
        std::string errorMsg = "Failed to initialize importer: " + std::string(error.Buffer());
        FbxErrorHandler::LogError(errorMsg);

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            std::string versionMsg = "FBX SDK version: " + std::to_string(lSDKMajor) + "." + 
                                   std::to_string(lSDKMinor) + "." + std::to_string(lSDKRevision) +
                                   " | File version: " + std::to_string(lFileMajor) + "." + 
                                   std::to_string(lFileMinor) + "." + std::to_string(lFileRevision);
            FbxErrorHandler::LogError(versionMsg);
            lImporter->Destroy();
            throw FbxSdkException(FbxSdkException::INVALID_FILE_VERSION, versionMsg);
        }

        lImporter->Destroy();
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

void FbxSdkLibrary::GetMetaData(FbxScene* pScene, map<const char*, const char*>& MetaData)
{
	FbxDocumentInfo* sceneInfo = pScene->GetSceneInfo();
	if (sceneInfo)
	{
		MetaData.insert(pair<const char*, const char*>("Title",sceneInfo->mTitle.Buffer()));
		MetaData.insert(pair<const char*, const char*>("Subject",sceneInfo->mSubject.Buffer()));
		MetaData.insert(pair<const char*, const char*>("Author",sceneInfo->mAuthor.Buffer()));
		MetaData.insert(pair<const char*, const char*>("Keywords",sceneInfo->mKeywords.Buffer()));
		MetaData.insert(pair<const char*, const char*>("Revision",sceneInfo->mRevision.Buffer()));
		MetaData.insert(pair<const char*, const char*>("Comment",sceneInfo->mComment.Buffer()));
	}
}

map<uint64_t, FbxGeometryInfo> FbxSdkLibrary::GetFbxGeometries(FbxScene* const pScene)
{
	map<uint64_t,FbxGeometryInfo> Geometries;
	
	if(!pScene)
	{
		FbxErrorHandler::LogError("FbxScene is null");
		return Geometries;
	}
	//Geometry参数
	FBXSDK_printf("FbxScene is right,BeginConverter\n");
	//三角化 - 只创建一次转换器
	FbxGeometryConverter converter(pScene->GetFbxManager());
	
	// 预先获取几何体数量
	const int geometryCount = pScene->GetGeometryCount();
	
	// 使用正向循环，避免混淆
	for(int i = 0; i < geometryCount; ++i)
	{
		FbxGeometry* geometry = pScene->GetGeometry(i);
		if(!geometry || geometry->GetAttributeType() != FbxNodeAttribute::eMesh)
			continue;
			
		FbxMesh* pMesh = static_cast<FbxMesh*>(geometry);
		
		// 如果不是三角网格，进行三角化
		if(!pMesh->IsTriangleMesh())
		{
			FBXSDK_printf("Triangulating mesh %d\n", i);
			// 三角化并替换原始网格
			FbxMesh* triangulatedMesh = converter.TriangulateMesh(pMesh);
			if(triangulatedMesh && triangulatedMesh != pMesh)
			{
				pMesh = triangulatedMesh;
			}
		}
		
		//Id用Scene获得的Id就不会错
		const uint64_t ID = pMesh->GetUniqueID();
		Geometries.insert(pair<uint64_t,FbxGeometryInfo>(ID,FbxGeometryInfo()));
		FbxGeometryInfo& GeometryInfo = Geometries[ID];
		
		
		int j,k, PolygonCount = pMesh->GetPolygonCount();
		//ControlPoints
		GetMeshControlPoint(pMesh,GeometryInfo.ControlPoints);
	
		FbxSection section;
			//三角面信息 - 预分配内存以提高性能
	vector<int> Triangle;
	Triangle.reserve(3 * PolygonCount);
	vector<FbxColor> Colors;
	Colors.reserve(3 * PolygonCount);
	vector<FbxVector2> UVs;
	UVs.reserve(3 * PolygonCount);
	vector<FbxVector4> Normals;
	Normals.reserve(3 * PolygonCount);
	vector<FbxVector4> Tangents;
	Tangents.reserve(3 * PolygonCount);
	vector<FbxVector4> Binormals;
	Binormals.reserve(3 * PolygonCount);
	vector<uint64_t> MaterialIds;
	MaterialIds.reserve(PolygonCount);
		//获得Polygon的信息
		for (j = 0;j<PolygonCount;++j)
		{
			int PolygonSize = pMesh->GetPolygonSize(j);
			 for(k =0;k<PolygonSize;++k )
			 {
			 	//获得三角面信息,小于0则意味着没找到对应的点
			 	int ControlPointIndex = pMesh->GetPolygonVertex(j, k);
			 	if(ControlPointIndex >=0)
			 	{
			 		Triangle.push_back(ControlPointIndex);
			 		Colors.push_back(GetPolygonVertexColor(pMesh,j,ControlPointIndex));
			 		FbxVector2 uv;
			 		GetPolygonUV(pMesh,j,ControlPointIndex,k,uv);
			 		UVs.push_back(uv);
			 	}
			 }
			 uint64_t MaterialId;
			 FbxVector4 normal, tangent, binormal;
			 GetPolygonNormal(pMesh,j,normal);
			 GetPolygonTangent(pMesh,j,tangent);
			 GetPolygonBinormal(pMesh,j,binormal);
			 // 为每个顶点添加相同的法线、切线和副法线
			 for(int v = 0; v < PolygonSize; ++v) {
			 	Normals.push_back(normal);
			 	Tangents.push_back(tangent);
			 	Binormals.push_back(binormal);
			 }
			 GetPolygonMaterialId(pMesh,j,MaterialId);
			 MaterialIds.push_back(MaterialId);
		}
		
		//根据材质分组
		map<uint64_t,FbxSection>* Sections = &GeometryInfo.Sections ;
		int vertexIndex = 0;
		for(int MatIndex = 0 ; MatIndex < (int)MaterialIds.size(); MatIndex++)
		{
			//如果当前材质没有记录先加入
			if(Sections->count(MaterialIds[MatIndex]) == 0)
			{
				Sections->insert(pair<uint64_t,FbxSection>(MaterialIds[MatIndex],FbxSection()));
			}
		
			FbxSection* Section = &(*Sections)[MaterialIds[MatIndex]];
			// 每个多边形有3个顶点（已经三角化）
			for(int l=0;l<3;++l)
			{
				if(vertexIndex < Triangle.size()) {
					Section->Triangle.push_back(Triangle[vertexIndex]);
					Section->Colors.push_back(Colors[vertexIndex]);
					Section->UVs.push_back(UVs[vertexIndex]);
					Section->Normals.push_back(Normals[vertexIndex]);
					Section->Tangents.push_back(Tangents[vertexIndex]);
					Section->Binormals.push_back(Binormals[vertexIndex]);
					vertexIndex++;
				}
			}
		}
	}
	
	return Geometries;
}

void FbxSdkLibrary::GetMeshControlPoint(const FbxMesh* pMesh,vector<FbxVector4>& ControlPoints)
{
	if(pMesh)
	{
		int lControlPointsCount = pMesh->GetControlPointsCount();
		FbxVector4* lControlPoints = pMesh->GetControlPoints();
		
		// 预分配内存以提高性能
		ControlPoints.reserve(lControlPointsCount);
		
		// 使用批量插入而不是逐个push_back
		ControlPoints.insert(ControlPoints.end(), lControlPoints, lControlPoints + lControlPointsCount);
	}
}

FbxColor FbxSdkLibrary::GetPolygonVertexColor(FbxMesh* pMesh, int PolygonIndex, int ControlPointIndex)
{
	FbxColor Color;
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
	return Color;
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
				Normal = ENolrmal->GetDirectArray().GetAt(PolygonIndex*3);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ENolrmal->GetIndexArray().GetAt(PolygonIndex*3);
					Normal = ENolrmal->GetDirectArray().GetAt(id);
					
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			// 移除危险的指针运算，这段代码原本试图设置连续的法线值，但实际上是错误的
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
				Tangent = ElTangent->GetDirectArray().GetAt(PolygonIndex*3);
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ElTangent->GetIndexArray().GetAt(PolygonIndex*3);
					Tangent = ElTangent->GetDirectArray().GetAt(id);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			// 移除危险的指针运算
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
					Binormal = ElBinormal->GetDirectArray().GetAt(PolygonIndex*3);
				}
				break;
			case FbxGeometryElement::eIndexToDirect:
				{
					int id = ElBinormal->GetIndexArray().GetAt(PolygonIndex*3);
					Binormal = ElBinormal->GetDirectArray().GetAt(id);
				}
				break;
			default:
				break; // other reference modes not shown here!
			}
			// 移除危险的指针运算
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
			MaterialInfo.Opacity.Factor = 1.0-MFbxDouble1.Get();

			// Display the Shininess
			MFbxDouble1 =((FbxSurfacePhong *) Material)->Shininess;
			MaterialInfo.Opacity.Factor = MFbxDouble1.Get();
            	
			// Display the Reflectivity
			MFbxDouble1 =((FbxSurfacePhong *) Material)->ReflectionFactor;
			MaterialInfo.Reflectivity.Factor = MFbxDouble1.Get();
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
			MaterialInfo.Opacity.Factor =  1.0-MFbxDouble1.Get();
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

const char* FbxSdkLibrary::test()
{
	
		FbxString AA = "AAAA";
		return AA.Buffer();
	
}

