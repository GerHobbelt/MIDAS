# MIDAS

<p>
  <a href="https://github.com/Stream-AD/MIDAS/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/License-Apache%202.0-blue.svg">
  </a>
</p>

This repo is forked from [Stream-AD/MIDAS](https://github.com/Stream-AD/MIDAS). The core is written in C++. This repo added a Python wrapper to it.

Blog post: 

## Requirements

*   Cmake
*   C++11
*   C++ standard libraries

## Installation

1. Make sure you clone this repo with `--recursive`
1. Open a terminal
1. `cd` to the project root `MIDAS/`
1. Install with pip: `pip install .`
1. Run folowwing code to test.
```python
from MIDAS import MIDAS, MIDASR, MIDASF


num_row = 2
num_col = 1024
factor = 0.5
threshold = 1e3

midas = MIDAS(num_row=num_row, num_col=num_col)
midas_r = MIDASR(num_row=num_row, num_col=num_col, factor=factor)
midas_f = MIDASF(num_row=num_row, num_col=num_col, threshold=threshold, factor=factor)

example_source = 3
example_destination = 5
example_timestamp = 1


score = midas.add_edge(source=example_source, destination=example_destination, timestamp=example_timestamp)
score_r = midas_r.add_edge(source=example_source, destination=example_destination, timestamp=example_timestamp)
score_f = midas_f.add_edge(source=example_source, destination=example_destination, timestamp=example_timestamp)
```

## Citation

If you use this code for your research, please consider citing the original authors' arXiv preprint

```bibtex
@misc{bhatia2020realtime,
    title={Real-Time Streaming Anomaly Detection in Dynamic Graphs},
    author={Siddharth Bhatia and Rui Liu and Bryan Hooi and Minji Yoon and Kijung Shin and Christos Faloutsos},
    year={2020},
    eprint={2009.08452},
    archivePrefix={arXiv},
    primaryClass={cs.LG}
}

```

or their AAAI paper


```bibtex
@inproceedings{bhatia2020midas,
    title="MIDAS: Microcluster-Based Detector of Anomalies in Edge Streams",
    author="Siddharth {Bhatia} and Bryan {Hooi} and Minji {Yoon} and Kijung {Shin} and Christos {Faloutsos}",
    booktitle="AAAI 2020 : The Thirty-Fourth AAAI Conference on Artificial Intelligence",
    year="2020"
}
```
