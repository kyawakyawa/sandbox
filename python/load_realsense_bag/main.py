import pyrealsense2 as rs
import numpy as np
import cv2
import os
import argparse


def extract_color_images_from_bag(bag_path, output_dir="output_images", skip: int = 1):
    skip = max(skip, 1)
    os.makedirs(output_dir, exist_ok=True)

    pipeline = rs.pipeline()
    config = rs.config()
    config.enable_device_from_file(bag_path, repeat_playback=False)

    # bagに含まれる全ストリームを有効化
    profile = pipeline.start(config)
    playback = profile.get_device().as_playback()
    playback.set_real_time(False)

    frame_count = 0

    try:
        while True:
            frames = pipeline.wait_for_frames()
            color_frame = frames.get_color_frame()
            if not color_frame:
                continue

            # カラーフォーマットを取得
            video_profile = color_frame.profile.as_video_stream_profile()
            fmt = video_profile.format()

            # ndarray化
            color_image = np.asanyarray(color_frame.get_data())

            # フォーマットに応じて変換
            if fmt == rs.format.rgb8:
                # RGB -> BGR
                color_image = cv2.cvtColor(color_image, cv2.COLOR_RGB2BGR)
            elif fmt == rs.format.yuyv:
                # YUY2 -> BGR
                color_image = cv2.cvtColor(color_image, cv2.COLOR_YUV2BGR_YUY2)
            # bgr8 の場合は変換不要

            # タイムスタンプからファイル名を作成
            timestamp_ms = color_frame.get_timestamp()
            filename = os.path.join(output_dir, f"color_{int(timestamp_ms)}.png")
            cv2.imwrite(filename, color_image)

            frame_count += 1

    except RuntimeError:
        pass
    finally:
        pipeline.stop()

    print(f"保存したフレーム数: {frame_count}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="bagファイルからカラー画像を取り出す")
    # 引数を追加
    parser.add_argument("--input_file", type=str, help="入力ファイルのパス")
    parser.add_argument("--output_dir", type=str, help="出力ディレクトリのパス")
    parser.add_argument(
        "--skip", type=int, default=0, help="スキップする数 (デフォルト: 0)"
    )

    # 引数をパース
    args = parser.parse_args()

    bag_file_path = args.input_file
    extract_color_images_from_bag(bag_file_path, output_dir=args.output_dir)
