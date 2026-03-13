import os
import shutil
import zipfile

def update_version_files(new_version, version_txt_path, release_folder):
    # 1. Update the version.txt used by the server
    with open(version_txt_path, 'w', encoding='utf-8') as f:
        f.write(new_version.strip())
    print(f"Updated server version file: {version_txt_path}")

    # 2. Copy that same version.txt into the release folder for the player
    release_version_path = os.path.join(release_folder, 'version.txt')
    shutil.copy2(version_txt_path, release_version_path)
    print(f"Copied version.txt to {release_folder}")

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
    print(f"Successfully created {output_zip}")

if __name__ == "__main__":
    version_txt = '../version.txt' # Path for GitHub Pages
    release_folder = 'release'      # Path where game.exe and Assets/ live
    zip_name = '../game.zip'        # Path for GitHub Pages download

    new_version = input("Enter new version number: ").strip()
    
    if new_version:
        update_version_files(new_version, version_txt, release_folder)
        zip_release(release_folder, zip_name)
    else:
        print("Operation aborted.")