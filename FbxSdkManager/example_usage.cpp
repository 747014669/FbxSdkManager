#include "FbxSdkWrapper.h"
#include "FbxSdkException.h"
#include <iostream>
#include <fstream>

/**
 * @brief 示例：使用改进后的FBX SDK库
 */
int main(int argc, char** argv)
{
    // 设置错误处理模式
    FbxErrorHandler::SetQuietMode(false);

    try
    {
        // 1. 创建FBX SDK包装器（自动管理资源）
        FbxSdkWrapper fbxWrapper;
        
        // 2. 加载FBX文件
        const char* filename = argc > 1 ? argv[1] : "test.fbx";
        std::cout << "Loading FBX file: " << filename << std::endl;
        
        if (!fbxWrapper.LoadFile(filename))
        {
            std::cerr << "Failed to load file: " << FbxErrorHandler::GetLastError() << std::endl;
            return -1;
        }

        // 3. 获取并打印元数据
        auto metadata = fbxWrapper.GetMetadata();
        std::cout << "\n=== Scene Metadata ===" << std::endl;
        for (const auto& pair : metadata)
        {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }

        // 4. 获取几何体信息
        auto geometries = fbxWrapper.GetGeometries();
        std::cout << "\n=== Geometries (" << geometries.size() << ") ===" << std::endl;
        
        for (const auto& geoPair : geometries)
        {
            std::cout << "Geometry ID: " << geoPair.first 
                      << ", Control Points: " << geoPair.second.ControlPoints.size()
                      << ", Materials: " << geoPair.second.Sections.size() << std::endl;

            // 转换为简化的网格数据
            auto simplifiedMeshes = FbxGeometryExporter::ConvertToSimplifiedMeshes(geoPair.second);
            
            for (size_t i = 0; i < simplifiedMeshes.size(); ++i)
            {
                const auto& mesh = simplifiedMeshes[i];
                std::cout << "  - Mesh " << i << ": "
                          << mesh.vertices.size() << " vertices, "
                          << mesh.indices.size() << " indices, "
                          << "Material ID: " << mesh.materialId << std::endl;
            }
        }

        // 5. 获取材质信息
        auto materials = fbxWrapper.GetMaterials();
        std::cout << "\n=== Materials (" << materials.size() << ") ===" << std::endl;
        
        for (const auto& matPair : materials)
        {
            const auto& mat = matPair.second;
            std::cout << "Material ID: " << matPair.first << std::endl;
            std::cout << "  - Diffuse: RGB(" 
                      << mat.Diffuse.Color[0] << ", "
                      << mat.Diffuse.Color[1] << ", " 
                      << mat.Diffuse.Color[2] << ")";
            if (mat.Diffuse.Texture)
            {
                std::cout << ", Texture: " << mat.Diffuse.Texture;
            }
            std::cout << std::endl;
            
            std::cout << "  - Opacity: " << mat.Opacity.Factor;
            if (mat.Opacity.Texture)
            {
                std::cout << ", Texture: " << mat.Opacity.Texture;
            }
            std::cout << std::endl;
        }

        // 6. 导出为自定义格式（示例）
        if (argc > 2)
        {
            std::string outputFile = argv[2];
            std::ofstream out(outputFile, std::ios::binary);
            
            if (out.is_open())
            {
                // 写入几何体数量
                uint32_t geoCount = static_cast<uint32_t>(geometries.size());
                out.write(reinterpret_cast<const char*>(&geoCount), sizeof(geoCount));
                
                // 写入每个几何体的简化数据
                for (const auto& geoPair : geometries)
                {
                    auto meshes = FbxGeometryExporter::ConvertToSimplifiedMeshes(geoPair.second);
                    uint32_t meshCount = static_cast<uint32_t>(meshes.size());
                    out.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
                    
                    for (const auto& mesh : meshes)
                    {
                        // 写入顶点数据
                        uint32_t vertexCount = static_cast<uint32_t>(mesh.vertices.size());
                        out.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
                        out.write(reinterpret_cast<const char*>(mesh.vertices.data()), 
                                 vertexCount * sizeof(FbxGeometryExporter::SimplifiedVertex));
                        
                        // 写入索引数据
                        uint32_t indexCount = static_cast<uint32_t>(mesh.indices.size());
                        out.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
                        out.write(reinterpret_cast<const char*>(mesh.indices.data()), 
                                 indexCount * sizeof(uint32_t));
                        
                        // 写入材质ID
                        out.write(reinterpret_cast<const char*>(&mesh.materialId), sizeof(mesh.materialId));
                    }
                }
                
                out.close();
                std::cout << "\nExported to: " << outputFile << std::endl;
            }
        }

        std::cout << "\nProcessing completed successfully!" << std::endl;
    }
    catch (const FbxSdkException& e)
    {
        std::cerr << "FBX SDK Exception: " << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}