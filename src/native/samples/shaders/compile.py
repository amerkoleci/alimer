import glob, os, enum
import argparse

parser = argparse.ArgumentParser(description='Compile all .hlsl shaders')
parser.add_argument('--dxc', type=str, help='path to DXC executable')
args = parser.parse_args()


def findDXC():
    def isExe(path):
        return os.path.isfile(path) and os.access(path, os.X_OK)

    if args.dxc != None and isExe(args.dxc):
        return args.dxc

    exe_name = "dxc"
    if os.name == "nt":
        exe_name += ".exe"

    full_path = os.path.join(os.getcwd(), exe_name)
    if isExe(full_path):
        return full_path

    for exe_dir in os.environ["PATH"].split(os.pathsep):
        full_path = os.path.join(exe_dir, exe_name)
        if isExe(full_path):
            return full_path

    sys.exit("Could not find DXC executable on PATH, and was not specified with --dxc")

dxc_path = findDXC()
#dxc_path = os.path.join(os.getcwd(), "../../tools/windows/dxc.exe")
print(dxc_path)

def compile_and_log_status(command, file):
    compile_status = os.popen(command).read()

    if compile_status:
        print(file, " -> ", compile_status)

class ShaderTypes(enum.Enum):
    none = 0
    vertex = 1
    pixel = 2
    compute = 3
    vertexAndPixel = 4

if __name__ == "__main__":
    # TODO
    root_signature_extracted = True

    # Create Compiled folder
    compiledShadersFolder = os.getcwd()
    #compiledShadersFolder = os.path.join(os.getcwd(), "Compiled")
    #if not os.path.exists(compiledShadersFolder):
    #    os.mkdir(compiledShadersFolder)

    # Compile shaders.

    vulkanConstantShift = 0
    vulkanTextureShift = 1000
    vulkanUavShift  = 2000
    vulkanSamplerShift  = 3000
    spirvArgs = "-spirv -fspv-target-env=vulkan1.2 -fvk-use-dx-layout -fvk-use-dx-position-w"
    spirvArgs = spirvArgs + " -fvk-b-shift " + str(vulkanConstantShift) + " 0"
    spirvArgs = spirvArgs + " -fvk-t-shift " + str(vulkanTextureShift) + " 0"
    spirvArgs = spirvArgs + " -fvk-u-shift " + str(vulkanUavShift) + " 0"
    spirvArgs = spirvArgs + " -fvk-s-shift " + str(vulkanSamplerShift) + " 0"
    spirvArgs = spirvArgs + " "

    for file in glob.glob("**/*.hlsl", recursive=True):
        shaderType = ShaderTypes.none

        # If the file has no VS, PS, or CS before the ., it is assumed to be a shader which has both PsMain and VsMain in the same file.
        if file.find("VS") == -1 and file.find("PS") == -1 and file.find("CS") == -1:
            shaderType = ShaderTypes.vertexAndPixel

            command = dxc_path + " -HV 2021 -T vs_6_1 -E vertexMain " + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + "Vertex.cso")
            compile_and_log_status(command, file)
            
            command = dxc_path + " -HV 2021 -T ps_6_1 -E fragmentMain " + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + "Fragment.cso")
            compile_and_log_status(command, file)

            # SPIRV
            command = dxc_path + " -HV 2021 -T vs_6_1 -E vertexMain " + " -D VULKAN " + spirvArgs + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + "Vertex.spv")
            compile_and_log_status(command, file)
            
            command = dxc_path + " -HV 2021 -T ps_6_1 -E fragmentMain " + " -D VULKAN " + spirvArgs + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + "Fragment.spv")
            compile_and_log_status(command, file)


        # Compile files which has either VS, CS, or PS postifx.
        elif file.find("VS") != -1:
            shaderType = ShaderTypes.vertex

            command = dxc_path + " -HV 2021 -T vs_6_1 -E vertexMain " + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + ".cso")
            compile_and_log_status(command, file)

        elif file.find("PS") != -1:
            shaderType = ShaderTypes.pixel

            command = dxc_path + " -HV 2021 -T ps_6_1 -E fragmentMain " + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + ".cso")
            compile_and_log_status(command, file)

        elif file.find("CS") != -1:
            shaderType = ShaderTypes.compute

            command = dxc_path + " -HV 2021 -T cs_6_1 -E main " + file + " -Fo " + os.path.join(compiledShadersFolder, file.split(".")[0] + ".cso")
            compile_and_log_status(command, file)
            
        if not root_signature_extracted:
            if shaderType == ShaderTypes.vertexAndPixel or ShaderTypes == ShaderTypes.vertex:
                command = dxc_path + " -T vs_6_1 -E vertexMain " + file + " -extractrootsignature -Fo " + "BindlessRS.cso"
            elif shaderType == ShaderTypes.pixel:
                command = dxc_path + " -T ps_6_1 -E fragmentMain " + file + " -extractrootsignature -Fo " + "BindlessRS.cso"
            elif shaderType == ShaderTypes.compute:
                continue
              
            compile_and_log_status(command, file)

            root_signature_extracted = True