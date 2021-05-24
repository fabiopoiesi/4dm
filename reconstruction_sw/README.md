# 3D pose and volumetric reconstruction

Here we describe the procedure to execute our reconstruction software.
We did our best to keep the code self-contained.
We provide pre-process data to reproduce our results.
The directory should have the following structure after downloading and extracting 2D skeleton and silhouette data:

    .
    +-- _code
    +-- _data
    |   +-- _poses
    |   +-- _silhouettes
    +-- _third_party
    |   +-- _sba-1.6
    +-- README.md

## Tested with

- Ubuntu 20.04
- Python 3.8
- GCC 9.3
- [Open3D 0.11.2](http://www.open3d.org/docs/release/index.html)
- [OpenCV 4.5.1.48](https://opencv.org/releases/)

## Setup Python environment

```
pip install -r requirements.txt
```

## System dependencies

The following dependecies are required to recompile sparse bundle asjustment libraries ([link](http://users.ics.forth.gr/~lourakis/sba/)).

sudo apt install liblapack-dev

sudo apt install libf2c2-dev


## Sparse bundle adjustment

Sparse bundle adjustment libraries should work under Ubuntu 20.04 without recompiling them.
To recompile them for another operating system follow this procedure.

1) Make sure the system dependecies listed above are installed.

2) Execute:

```
cd third_party
cd sba-1.6
make
cd demo
make
cp libmysba.so ../../../code/
cd ..
cp libsba.so ../../code/

```

## How to use the scripts

Change the *dataset_root_folder* within each python script to point to the root folder of the extracted [dataset](https://drive.google.com/file/d/1AvkGph7TXxsxoqQXEVZErHHllutC4Ncc/view?usp=sharing) archive.

The directory *code* contains four python scripts:

1) *gen_viz_skeleton.py*: to reconstruct and visualise the 3D pose of the actor using pre-processed 2D skeletons on each image plane. To execute it:

```
cd code
python python gen_viz_skeleton.py
```

2) *gen_volumetric.py* & *viz_volumetric.py*: to reconstruct and visualise the volumetric reconstruction of the actor using pre-processed 2D silhouettes on each image plane.
Because shape-from-silhouette is computationally costly, we first generate the meshes (.ply files) using *gen_volumetric.py* and then we visualise them using *viz_volumetric.py*. To execute the reconstruction:

```
cd code
python gen_volumetric.py
```

The ply files are stored in *results/volumetric_meshes*. To visualise the reconstructions:

```
python viz_volumetric.py
```

3) *utils.py*: used only to improve visualisation.

When the visualisation window of the scripts appears, choose the desired viewpoint with the mouse and press *q*.


## 2D skeleton and silhouette data

2D skeleton data is computed using [OpenPose](https://github.com/CMU-Perceptual-Computing-Lab/openpose), while silhouette data is computed using [PointRend](https://github.com/facebookresearch/detectron2/tree/master/projects/PointRend).
For convenience, we provide the pre-procecessed data at this [link](https://drive.google.com/file/d/1NwX-x-ZaskilKmQgtiimrv0-EKSXcfVj/view?usp=sharing).

Download the archive and unzip is inside the main directory:

```
unzip data.zip
```

## Citing our work

Please cite the following paper if you use our code or our dataset:

```latex
@article{Bortolon2021,
    title = {Multi-view data capture for dynamic object reconstruction using handheld augmented reality mobiles},
    author = {Bortolon, Matteo and Bazzanella, Luca and Poiesi, Fabio},
    journal = {Journal of Real-Time Image Processing},
    volume = {18},
    pages = {345–355},
    month = {Mar},
    year = {2021}
}
```





