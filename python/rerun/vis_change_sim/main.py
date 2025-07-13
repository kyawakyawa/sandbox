import tyro
from dataclasses import dataclass

import rerun as rr  # pip install rerun-sdk
import rerun.blueprint as rrb

import numpy as np


@dataclass
class Config:
    trajectory_path: str = "./trajectory.txt"

    all_frame: bool = False
    draw_points: bool = True

    host: str = "0.0.0.0:9876"


WIDTH = 640
HEIGHT = 480
FX = 320
FY = 320
CX = 320
CY = 240


def qvec2rotmat(qvec):
    return np.array(
        [
            [
                1 - 2 * qvec[2] ** 2 - 2 * qvec[3] ** 2,
                2 * qvec[1] * qvec[2] - 2 * qvec[0] * qvec[3],
                2 * qvec[3] * qvec[1] + 2 * qvec[0] * qvec[2],
            ],
            [
                2 * qvec[1] * qvec[2] + 2 * qvec[0] * qvec[3],
                1 - 2 * qvec[1] ** 2 - 2 * qvec[3] ** 2,
                2 * qvec[2] * qvec[3] - 2 * qvec[0] * qvec[1],
            ],
            [
                2 * qvec[3] * qvec[1] - 2 * qvec[0] * qvec[2],
                2 * qvec[2] * qvec[3] + 2 * qvec[0] * qvec[1],
                1 - 2 * qvec[1] ** 2 - 2 * qvec[2] ** 2,
            ],
        ]
    )


def rotmat2qvec(R):
    Rxx, Ryx, Rzx, Rxy, Ryy, Rzy, Rxz, Ryz, Rzz = R.flat
    K = (
        np.array(
            [
                [Rxx - Ryy - Rzz, 0, 0, 0],
                [Ryx + Rxy, Ryy - Rxx - Rzz, 0, 0],
                [Rzx + Rxz, Rzy + Ryz, Rzz - Rxx - Ryy, 0],
                [Ryz - Rzy, Rzx - Rxz, Rxy - Ryx, Rxx + Ryy + Rzz],
            ]
        )
        / 3.0
    )
    eigvals, eigvecs = np.linalg.eigh(K)
    qvec = eigvecs[[3, 0, 1, 2], np.argmax(eigvals)]
    if qvec[0] < 0:
        qvec *= -1
    return qvec


def inverse_pose(txyz: np.ndarray, qxyzw: np.ndarray):
    mat44 = np.eye(4)
    qwxyz = qxyzw[[3, 0, 1, 2]]

    mat44[:3, :3] = qvec2rotmat(qwxyz)
    mat44[:3, 3] = txyz

    mat44 = np.linalg.inv(mat44)

    txyz = mat44[:3, 3]
    qwxyz = rotmat2qvec(mat44[:3, :3])
    qxyzw = qwxyz[[1, 2, 3, 0]]

    return txyz, qxyzw


def visualize_all_frame(
    idxs: list[str],
    txyzs: list[np.ndarray],
    qxyzws: list[np.ndarray],
):
    # for idx, txyz, qxyzw in zip(idxs, txyzs, qxyzws):
    for i, (txyz, qxyzw) in enumerate(zip(txyzs, qxyzws)):
        idx = str(i)
        rr.log(
            "/all/camera-" + idx,
            rr.Transform3D(
                translation=txyz,
                rotation=rr.Quaternion(xyzw=qxyzw),
                from_parent=True,
                axis_length=0.02,
            ),
        )
        # rr.log("camera", rr.ViewCoordinates.RDF, static=True)

        rr.log(
            "/all/camera-" + idx + "/image",
            rr.Pinhole(
                resolution=[WIDTH, HEIGHT],
                focal_length=[WIDTH / 5, WIDTH / 5],
                principal_point=[WIDTH / 2, HEIGHT / 2],
                image_plane_distance=0.02,
            ),
        )


def visualize(config: Config):
    # Read and process the trajectory.txt file
    idx: list[str] = []
    txyzs: list[np.ndarray] = []
    qxyzws: list[np.ndarray] = []
    with open(config.trajectory_path, "r") as file:
        for line in file:
            # Split each line by spaces
            data = line.strip().split()
            # Process the split data
            print(data)  # Replace this with actual processing logic

            idx.append(data[0])
            txyz = np.array([float(data[i]) for i in [1, 2, 3]])
            qxyzw = np.array([float(data[i]) for i in [4, 5, 6, 7]])
            txyz, qxyzw = inverse_pose(txyz, qxyzw)

            txyzs.append(txyz)
            qxyzws.append(qxyzw)

    for txyz, qxyzw in zip(txyzs, qxyzws):
        rr.log(
            "/camera",
            rr.Transform3D(
                translation=txyz,
                rotation=rr.Quaternion(xyzw=qxyzw),
                from_parent=True,
                axis_length=0.1,
            ),
        )
        rr.log("/camera", rr.ViewCoordinates.RDF, static=True)
        rr.log(
            "/camera/image",
            rr.Pinhole(
                resolution=[WIDTH, HEIGHT],
                focal_length=[FX, FY],
                principal_point=[CX, CY],
                image_plane_distance=0.1,
            ),
        )
    if config.all_frame:
        visualize_all_frame(idx, txyzs, qxyzws)


def main() -> None:
    """
    parser = argparse.ArgumentParser(description="")
    parser.add_argument("--dataset", required=True, type=str)
    parser.add_argument("--image_dir", required=False, default="", type=str)
    parser.add_argument(
        "--input_type",
        type=str,
        default="BIN",
        choices=["BIN", "TXT"],
        help="input type",
    )
    parser.add_argument("--all_frames", action="store_true")
    rr.script_add_args(parser)
    args = parser.parse_args()

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
        rrb.Spatial2DView(name="Camera", origin="/camera/image"),
        row_shares=[3, 2],
    )

    rr.script_setup(args, "rerun_vis_colmap_sparse", default_blueprint=blueprint)

    visualize(args)
    """

    config = tyro.cli(Config)

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
        rrb.Spatial2DView(name="Camera", origin="/camera/image"),
        row_shares=[3, 2],
    )
    rr.init("rerun_vis_change_sim", default_blueprint=blueprint, spawn=False)

    rr.connect(config.host)
    try:
        visualize(config)
    finally:
        rr.disconnect()


if __name__ == "__main__":
    main()
