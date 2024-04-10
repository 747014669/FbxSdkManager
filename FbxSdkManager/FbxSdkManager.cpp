#include <fbxsdk.h>

#include "FbxSdkLibrary.h"

/* Tab character ("\t") counter */
int numTabs = 0;
vector<FbxNodeInfo> NodeInfos;
/**
 * Print the required number of tabs.
 */
void PrintTabs() {
    for (int i = 0; i < numTabs; i++)
        printf("\t");
}

/**
 * Return a string-based representation based on the attribute type.
 */
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
    switch (type) {
    case FbxNodeAttribute::eUnknown: return "unidentified";
    case FbxNodeAttribute::eNull: return "null";
    case FbxNodeAttribute::eMarker: return "marker";
    case FbxNodeAttribute::eSkeleton: return "skeleton";
    case FbxNodeAttribute::eMesh: return "mesh";
    case FbxNodeAttribute::eNurbs: return "nurbs";
    case FbxNodeAttribute::ePatch: return "patch";
    case FbxNodeAttribute::eCamera: return "camera";
    case FbxNodeAttribute::eCameraStereo: return "stereo";
    case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
    case FbxNodeAttribute::eLight: return "light";
    case FbxNodeAttribute::eOpticalReference: return "optical reference";
    case FbxNodeAttribute::eOpticalMarker: return "marker";
    case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
    case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
    case FbxNodeAttribute::eBoundary: return "boundary";
    case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
    case FbxNodeAttribute::eShape: return "shape";
    case FbxNodeAttribute::eLODGroup: return "lodgroup";
    case FbxNodeAttribute::eSubDiv: return "subdiv";
    default: return "unknown";
    }
}

/**
 * Print an attribute.
 */
void PrintAttribute(FbxNodeAttribute* pAttribute) {
    if (!pAttribute) return;

    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();
    PrintTabs();
    // Note: to retrieve the character array of a FbxString, use its Buffer() method.
    printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

/**
 * Print a node, its attributes, and all its children recursively.
 */
void PrintNode(FbxNode* pNode) {
    
    for(int i = 0; i < pNode->GetChildCount(); i++)
    {
       PrintNode(pNode->GetChild(i));
        
    }
    FbxNodeInfo GeometryInfo;
    GeometryInfo.NodeName = pNode->GetName();
    GeometryInfo.Id = pNode->GetUniqueID();
    if(pNode->GetNodeAttribute())
    {
        if(pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            if(pNode->GetParent())
            {
                GeometryInfo.ParentId = pNode->GetParent()->GetUniqueID();
            }
            GeometryInfo.LinkMeshId = pNode->GetMesh()->GetUniqueID();
            for(int i = 0;i<pNode->GetMaterialCount();++i)
            {
                GeometryInfo.LinkMaterialsId.push_back(pNode->GetMaterial(i)->GetUniqueID());
            }
        }
    }
    NodeInfos.push_back(GeometryInfo);
    FBXSDK_printf("Parent:%llu,SelfId:%llu,Name:%s,MeshId:%llu,MaterialsNum:%llu \n",GeometryInfo.ParentId,GeometryInfo.Id,GeometryInfo.NodeName,GeometryInfo.LinkMeshId,GeometryInfo.LinkMaterialsId.size());
}

/**
 * Main function - loads the hard-coded fbx file,
 * and prints its contents in an xml format to stdout.
 */
int main(int argc, char** argv) {

    // Change the following filename to a suitable filename value.
    const char* lFilename = R"(C:\Users\Administrator\Desktop\Instance.fbx)";

    // Initialize the SDK manager. This object handles all our memory management.
    FbxManager* lSdkManager = FbxManager::Create();

    // Create the IO settings object.
    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    // Create an importer using the SDK manager.
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

    // Use the first argument as the filename for the importer.
    if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }

    // Create a new scene so that it can be populated by the imported file.
    FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
    
    // Import the contents of the file into the scene.
    lImporter->Import(lScene);
    // The file is imported; so get rid of the importer.
    lImporter->Destroy();
   
    for(int i= 0;i<lScene->GetGeometryCount();i++)
    {
        FBXSDK_printf("Name:%s,ID:%llu \n",lScene->GetGeometry(i)->GetName(),lScene->GetGeometry(i)->GetUniqueID());
    }


    int k = 0;
    map<uint64_t,FbxGeometryInfo> Geometries = FbxSdkLibrary::GetFbxGeometries(lScene);
    for(auto it = Geometries.begin();it!=Geometries.end();it++)
    {
 
        FBXSDK_printf("GeometryId:%llu,MeshControlPointsize:%llu,SectionNum:%llu \n",it->first,it->second.ControlPoints.size(),it->second.Sections.size());
    }
        
    // map<uint64_t,FbxMaterialsInfo> MaterialInfos;
    // FbxSdkLibrary::GetFbxMaterials(lScene,MaterialInfos);
    // for(auto it = MaterialInfos.begin();it!=MaterialInfos.end();it++)
    // {
    //     FBXSDK_printf("Id:%llu,material:%s\n",it->first,it->second.Diffuse.Texture);
    // }
    // Print the nodes of the scene and their attributes recursively.
    // Note that we are not printing the root node because it should
    // not contain any attributes.
    // FbxNode* lRootNode = lScene->GetRootNode();
    // if (lRootNode) {
    //     for (int i = 0; i < lRootNode->GetChildCount(); i++)
    //         PrintNode(lRootNode->GetChild(i));
    //}
    // Destroy the SDK manager and all the other objects it was handling.
    lSdkManager->Destroy();
    system("Pause");
    return 0;
}