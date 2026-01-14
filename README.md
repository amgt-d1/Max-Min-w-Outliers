## Introduction
* This repository provides implementations of our algorithms for Max-Min Diversification with Outliers.
* As for the details about these algorithms, please read our AAAI2023 paper, [Diversity Maximization in the Presence of Outliers](https://ojs.aaai.org/index.php/AAAI/article/view/26454/26226).

## Requirement
* We used a Ubuntu 20.04 LTS machine and our codes were compiled by g++ 9.4.0.

## How to use
* Compile:
    * g++ -o xx.out main.cpp -O3
    * xx can be arbitrary.
* Run:
    * ./xx.out
* Algorithms:
   * Greedy: our greedy algorithm
   * Stream: our streaming algorithm without coreset
   * Stream-coreset: our streaming algorithm with coreset

## Datasets
* "_dataset" directory has to be located at `src/_dataset` (i.e., src/_dataset/xx.csv, where xx is a dataset name). 
* A dataset has to be a csv file.
   * Each line represents a data item, and, under a d-dimensional vector assumption, each dimension has to be separeted by comma.<br>
     For example, a 3-dimensional dataset (of two vectors) has to be:<br>
     ``` 
     1,2,3
     4,5,6
     ``` 
      
   * dimensionality is set in `file_input.hpp`.

## Parameters
* Set some value in the corresponding txt file in `_parameter`.

## Citation
If you use our implementation, please cite the following paper.
``` 
@inproceedings{amagata2023diversity,  
    title={Diversity Maximization in the Presence of Outliersh},  
    author={Amagata, Daichi},  
    booktitle={AAAI},  
    pages={12338--12345},  
    year={2023}  
}
```

## License
Copyright (c) 2022 Daichi Amagata  
This software is released under the [MIT license](https://github.com/amgt-d1/Max-Min-w-Outliers/blob/main/license.txt).
