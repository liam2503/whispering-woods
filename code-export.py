import os

def compile_codebase(output_filename="code-export.txt"):
    allowed_extensions = (".cpp", ".hpp", ".h", ".c")
    allowed_names = ("Makefile", "makefile", "CMakeLists.txt")
    
    with open(output_filename, 'w', encoding='utf-8') as outfile:
        for root, _, files in os.walk('.'):
            for file in files:
                if file.endswith(allowed_extensions) or file in allowed_names:
                    if file == output_filename:
                        continue
                        
                    filepath = os.path.join(root, file)
                    try:
                        with open(filepath, 'r', encoding='utf-8') as infile:
                            content = infile.read()
                            outfile.write(f"========== FILE: {filepath} ==========\n")
                            outfile.write(content)
                            outfile.write(f"\n========== END OF {filepath} ==========\n\n")
                    except Exception as e:
                        outfile.write(f"========== ERROR READING {filepath}: {e} ==========\n\n")

if __name__ == "__main__":
    compile_codebase()