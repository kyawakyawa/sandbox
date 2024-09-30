import numpy as np


def create_simple_case():
    cam_a_cen_world = np.array([[0, 0, 0], [1, 0, 0], [1, -1, 0], [0, -1, 0]])
    cam_b_cen_world = np.array([[0, -1, 1], [0, -1, 3], [0, -3, 3], [0, -3, 1]])

    # rotations are common for all cameras
    cam_a_rot = np.array([[-1, 0, 0], [0, -1, 0], [0, 0, 1]])[None, :, :].repeat(
        4, axis=0
    )
    cam_b_rot = np.array([[0, -1, 0], [0, 0, -1], [1, 0, 0]])[None, :, :].repeat(
        4, axis=0
    )

    return cam_a_rot, cam_a_cen_world, cam_b_rot, cam_b_cen_world


def create_simple_points(n: int, scale: float, rot: np.ndarray, trans: np.ndarray):

    # 0 < x < 1
    # -1 < y < 0
    # 0.5 < z < 1.5
    # で点をn個作成
    points = np.random.rand(n, 3)
    points_applied_sim3 = (scale * rot @ points[:, None] + trans[:, None]).squeeze(
        axis=-1
    )

    return points, points_applied_sim3
