import os
import re
import zipfile

def get_current_version(main_cpp_path):
    try:
        with open(main_cpp_path, 'r', encoding='utf-8') as f:
            content = f.read()
        match = re.search(r'checkForUpdates\("([^"]+)"\)', content)
        if match:
            return match.group(1)
    except FileNotFoundError:
        pass
    return "Unknown"

def update_version(new_version, main_cpp_path, version_txt_path):
    try:
        with open(main_cpp_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        new_content = re.sub(r'(checkForUpdates\(")[^"]+("\))', rf'\g<1>{new_version}\g<2>', content)

        with open(main_cpp_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Updated {main_cpp_path} to version {new_version}")

        with open(version_txt_path, 'w', encoding='utf-8') as f:
            f.write(new_version.strip())
        print(f"Updated {version_txt_path} to version {new_version}")

    except Exception as e:
        print(f"Error updating files: {e}")

def zip_release(release_dir, output_zip):
    if not os.path.exists(release_dir):
        print(f"Error: Directory '{release_dir}' does not exist.")
        return

    with zipfile.ZipFile(output_zip, 'w', zipfile.ZIP_DEFLATED) as zf:
        for root, _, files in os.walk(release_dir):
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, release_dir)
                zf.write(file_path, arcname)
    print(f"Successfully created {output_zip} from the contents of {release_dir}")

if __name__ == "__main__":
    main_cpp = 'src/main.cpp'
    version_txt = '../version.txt'
    release_folder = 'release'
    zip_name = '../game.zip'

    current_version = get_current_version(main_cpp)
    print(f"Current version: {current_version}")

    new_version = input("Enter new version number: ").strip()
    
    if new_version:
        update_version(new_version, main_cpp, version_txt)
        zip_release(release_folder, zip_name)
    else:
        print("Operation aborted. No version entered.")