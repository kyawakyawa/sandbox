import os
import argparse
from pathlib import Path
from read_write_model import read_images_binary, read_images_text


def find_png_files(root_dir):
    png_files = []
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith(".png"):
                # 相対パスに変換してリストに追加
                rel_path = os.path.relpath(os.path.join(dirpath, filename), root_dir)
                png_files.append(rel_path)
    return png_files


def read_colmap_images(sparse_dir):
    """
    Read COLMAP images from sparse_dir.
    Try to read images.txt first, then images.bin if images.txt doesn't exist.
    """
    images_txt_path = os.path.join(sparse_dir, "images.txt")
    images_bin_path = os.path.join(sparse_dir, "images.bin")

    if os.path.exists(images_txt_path):
        print(f"Reading images from {images_txt_path}")
        return read_images_text(Path(images_txt_path))
    elif os.path.exists(images_bin_path):
        print(f"Reading images from {images_bin_path}")
        return read_images_binary(Path(images_bin_path))
    else:
        raise FileNotFoundError(
            f"Neither images.txt nor images.bin found in {sparse_dir}"
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Find PNG files in a directory.")
    parser.add_argument(
        "--images_dir",
        type=str,
        required=True,
        help="Root directory to search for PNG files",
    )
    parser.add_argument(
        "--sparse_dir", type=str, required=True, help="Colmap sparse directory"
    )
    args = parser.parse_args()

    # Find all PNG files in the images directory
    png_list = find_png_files(args.images_dir)
    print(f"Found {len(png_list)} PNG files in {args.images_dir}")

    # Read COLMAP images from sparse_dir
    try:
        colmap_images = read_colmap_images(args.sparse_dir)
        print(f"Found {len(colmap_images)} images in COLMAP reconstruction")

        colmap_images = set(
            img.name for img in colmap_images.values()
        )  # Extract image names from COLMAP images
        
        # Find PNG files that are not in the COLMAP reconstruction
        failed_images = []
        for png_file in png_list:
            # Remove file extension for comparison with COLMAP image names
            png_name = png_file # os.path.splitext(png_file)[0]
            if png_name not in colmap_images:
                failed_images.append(png_file)
        
        print(f"\nFound {len(failed_images)} PNG files not in COLMAP reconstruction:")
        for img in failed_images:
            print(f"  - {img}")
            
    except FileNotFoundError as e:
        print(f"Error: {str(e)}")
        exit(1)

    
