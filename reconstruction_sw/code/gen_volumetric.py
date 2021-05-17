import open3d as o3d
import cv2 as cv
import numpy as np
import quaternion as quat
import glob
import os
import json
import mcubes

do_viz = False

dataset_root_folder = '/home/poiesi/data/datasets/4dm/'
open_pose_data = '../data/poses'
silhouette_dir = '../data/silhouettes'
mesh_res_dir = '../results/volumetric_meshes'
dataset_name = 'easy'

if not os.path.isdir(mesh_res_dir):
    os.makedirs(mesh_res_dir)
if not os.path.isdir(os.path.join(mesh_res_dir, dataset_name)):
    os.mkdir(os.path.join(mesh_res_dir, dataset_name))

mobiles = {1: '4846a8bc1c6f287a725111d3f60f2385',
           2: 'cc74017ee8081c07917f03332f92ede6',
           3: 'd7ec5aa63b32a4878ca037e2a7fec252',
           4: '172673834746e6d591b4b00599c98ffa',
           5: '378d43cea3872b0551900a15c92e0ec4',
           6: '642a3152b339263852fffe527a9de347'}

# corrections computed with https://github.com/fabiopoiesi/dip
pose_corrections = [[[ 1., 0., 0., 0.],
                     [ 0., 1., 0., 0.],
                     [ 0., 0., 1., 0.],
                     [ 0., 0., 0., 1.]],
                    [[ 0.99905457, -0.02676729, -0.0342561,   0.15728716],
                     [ 0.02700194,  0.99961486,  0.00640553, -0.01462191],
                     [ 0.03407145, -0.00732446,  0.99939256,  0.00472023],
                     [ 0.,          0.,          0.,          1.,        ]],
                    [[ 0.99960058, -0.00509856,  0.02779724,  0.14406139],
                     [ 0.00464171,  0.9998535,   0.01647517, -0.00341851],
                     [-0.02787717, -0.01633956,  0.9994778,  -0.03475881],
                     [ 0.,          0.,         0.,          1.,        ]],
                    [[ 9.98765432e-01, -4.96677155e-02, -8.54867548e-04,  1.72896007e-01],
                     [ 4.96718223e-02,  9.98749038e-01,  5.75062066e-03, -6.17926371e-02],
                     [ 5.68177950e-04, -5.78598396e-03,  9.99983100e-01,  1.00602630e-01],
                     [ 0.00000000e+00,  0.00000000e+00,  0.00000000e+00,  1.00000000e+00]],
                    [[ 0.99998321, -0.00385142, -0.00432897,  0.19584433],
                     [ 0.00391902,  0.99986879,  0.01571766, -0.0033018 ],
                     [ 0.00426787, -0.01573436,  0.9998671,   0.0189917 ],
                     [ 0.,          0.,          0.,          1.        ]],
                    [[0.99847665, - 0.05491205,  0.00538861,  0.218133],
                     [0.05486804,  0.99846156,  0.00800104,  0.00688609],
                     [-0.00581968, - 0.00769319,  0.99995347, - 0.02124214],
                     [0., 0., 0., 1.]]
                    ]

mobile_host = 1

imgW = 640
imgH = 480

frame_list = glob.glob(os.path.join(dataset_root_folder, dataset_name, mobiles[mobile_host], '*.jpg'))
frame_list.sort()

number_of_frames = int(os.path.splitext(os.path.basename(frame_list[-1]))[0]) + 1

start_frame = 0
end_frame = number_of_frames

limbs = np.asarray([[0, 1], [0, 16], [0, 18], [0, 15], [15, 17], [1, 2], [1, 5], [5, 6], [6, 7], [2, 3], [3, 4], [1, 8],
                    [8, 12], [8, 9], [12, 13], [13, 14], [14, 21], [14, 19], [19, 20], [9, 10], [10, 11], [11, 24],
                    [11, 22], [22, 23]])

c = np.asarray([1.00563633, 0, -1.10522775])
b = .9
h = 1.9

# init mobile captures
mobile_captures = dict()
for mobile_number, mobile in mobiles.items():
    mobile_captures[mobile_number] = {'im': [],
                                      'position': [],
                                      'rotation': [],
                                      'principal_point': [],
                                      'focal_length': [],
                                      'K': [],
                                      'frustum': o3d.geometry.LineSet(),
                                      'person_pose': []}

# PREPARE DATA FOR SFS
voxel_res = 160
X, Y, Z = np.meshgrid(np.linspace(c[0] - b, c[0] + b, voxel_res),
                   np.linspace(0, -h, voxel_res),
                   np.linspace(c[2] - b, c[2] + b, voxel_res))

voxels_coords = np.asarray([X.flatten(), Y.flatten(), Z.flatten()]).T
voxels_coords_ones = np.c_[voxels_coords, np.ones((len(voxels_coords), 1))]

for fr in range(start_frame, end_frame):

    flag_skip_frame = 0  # frame skipping occurs when partial frames are detected

    ####################
    #################### load camera data and estimated poses for each mobile
    ####################

    for mobile_number, mobile in mobiles.items():

        jpg_file = os.path.join(dataset_root_folder, dataset_name, mobile, '%04d.jpg' % fr)
        json_file = os.path.join(dataset_root_folder, dataset_name, mobile, '%04d.bin' % fr)

        if not os.path.isfile(jpg_file):
            flag_skip_frame = 1
            continue

        # mobile pose
        with open(json_file, 'r') as jf:
            json_text = jf.read()
            meta = json.loads(json_text)

        pp = np.asarray([meta['principalPoint']['x'], meta['principalPoint']['y']])
        fl = np.asarray([meta['focalLength']['x'], meta['focalLength']['y']])

        rotation = quat.quaternion(meta['rotation']['w'],
                                   -meta['rotation']['x'],
                                   meta['rotation']['y'],
                                   -meta['rotation']['z'])

        centre = np.asarray([meta['position']['x'],
                             -meta['position']['y'],
                             meta['position']['z']])

        # http://ksimek.github.io/2012/08/22/extrinsic/
        K = np.asarray([[fl[0], 0, pp[0]],
                        [0, fl[1], pp[1]],
                        [0, 0, 1]])

        R = quat.as_rotation_matrix(rotation)

        mobile_captures[mobile_number]['principal_point'] = pp
        mobile_captures[mobile_number]['focal_length'] = fl
        mobile_captures[mobile_number]['quaternion'] = rotation
        mobile_captures[mobile_number]['rotation'] = R
        mobile_captures[mobile_number]['position'] = centre
        mobile_captures[mobile_number]['K'] = K

    if flag_skip_frame:
        continue

    # SHAPE FROM SILHOUETTE
    filled = []
    for mobile_number, mobile in mobile_captures.items():

        # organise camera parameters in a matrix
        K = mobile['K']
        R = quat.as_rotation_matrix(mobile['quaternion'])
        C = mobile['position']

        T = np.zeros((4, 4))
        T[:3, :3] = R
        T[:3, 3] = C
        T[3, 3] = 1

        Tnew = np.asarray(pose_corrections[mobile_number - 1]).T @ T

        R = Tnew[:3, :3]
        C = Tnew[:3, 3]

        P = np.dot(K, np.c_[R.T, - np.dot(R.T, C)])

        voxels_proj_2d = P @ voxels_coords_ones.T

        voxels_proj_2d /= voxels_proj_2d[2]

        silhouette_file = os.path.join(silhouette_dir, dataset_name, mobiles[mobile_number], '%04d.png' % fr)
        im_silhouette = cv.imread(silhouette_file, -1)
        im_silhouette[im_silhouette > 0] = 1

        voxels_proj_2d = np.round(voxels_proj_2d).astype(int)
        x_good = np.logical_and(voxels_proj_2d[0, :] > -1, voxels_proj_2d[0, :] < imgW)
        y_good = np.logical_and(voxels_proj_2d[1, :] > -1, voxels_proj_2d[1, :] < imgH)
        good = np.logical_and(x_good, y_good)
        indices = np.where(good)[0]
        fill = np.zeros(voxels_proj_2d.shape[1])
        sub_voxels_proj_2d = voxels_proj_2d[:2, indices]
        res = im_silhouette[sub_voxels_proj_2d[1, :], sub_voxels_proj_2d[0, :]]
        fill[indices] = res

        filled.append(fill)

    filled = np.vstack(filled)

    # the occupancy is computed as the number of camera in which the point "seems" not empty
    occupancy = np.sum(filled, axis=0)

    # Marching cubes
    occ = occupancy.reshape((voxel_res, voxel_res, voxel_res))
    vertices, triangles = mcubes.marching_cubes(occ, 5)

    v = vertices.astype(int)
    verts = np.c_[X[(v[:, 0], v[:, 1], v[:, 2])], Y[(v[:, 0], v[:, 1], v[:, 2])], Z[(v[:, 0], v[:, 1], v[:, 2])]]

    # Visualisation with open3d
    mesh = o3d.geometry.TriangleMesh()
    mesh.triangles = o3d.utility.Vector3iVector(triangles)
    mesh.vertices = o3d.utility.Vector3dVector(verts)

    mesh_out = mesh.filter_smooth_simple(number_of_iterations=3)
    mesh_out.compute_vertex_normals()

    o3d.io.write_triangle_mesh(os.path.join(mesh_res_dir, dataset_name, '{:04d}.ply'.format(fr)), mesh_out)
    print('{} mesh stored'.format(fr))

    if do_viz:
        o3d.visualization.draw_geometries([mesh_out])