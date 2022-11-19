## Introduction
* This repository provides implementations of our algorithms for Max-Min Diversification with Outliers.
* As for the details about these algorithms, please read our AAAI2023 paper, [Diversity Maximization in the Presence of Outliers](https://).

## Requirement
* We used a Ubuntu 20 LTS machine and our codes were compiled by g++ 9.4.0.

## How to use
* Compile:
    * g++ -o xx.out main.cpp -O3
    * xx can be arbitrary.
* Run:
    * ./xx.out

## Datasets
* "_dataset" directory has to be located at "src/_dataset" (i.e., src/_dataset/xx.csv, where xx is a dataset name). 

## Parameters
* Set some value in the corresponding txt file in `_parameter`.


## Citation
If you use our implementation, please cite the following paper.
``` 
@inproceedings{amagata2023diversity,  
    title={Diversity Maximization in the Presence of Outliersh},  
    author={Amagata, Daichi},  
    booktitle={AAAI},  
    pages={xx--xx},  
    year={2023}  
}
```

## License
Copyright (c) 2022 Daichi Amagata  
This software is released under the [MIT license](https://github.com/amgt-d1/Max-Min-w-Outliers/blob/main/license.txt).
