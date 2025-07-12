# MFT-Analysis

# How to use for dummysssssss.... jklmao

#TLDR:
    1. get any sort of OHCLV data (open low high close volume data)

    2. but lets assume you have OHCLV data, with the feature engineering module you can calculate up to 65 features for over 10k stocks within 2-5 mins (dependingon computer circumstances unless you have a bad laptop but either way its free and fast enough to be practical for most)

    3. after you can visualize the data with the visualization module

# Actual SHit instructions:

    # hOW TO FEATURE engineer 
    1. get any sort of OHCLV data (open low high close volume data)
    # Make sure you set the path main.cpp for your DATA input andoutput.

    2. install build:
        - feature_engineering/./build.sh install
    3. run build
        - feature_engineering/./build.sh run
    
    # if it dont work good luck, i only use a arm apple and a intel windows.

    # How to visualize the data 

    # hOW TO visualize
    1. after extracting the features from your ohclv data.

    2. install build:
        - ./build/StockVisualizer install
        - or ./build.sh install 

        # idk i am a vibe coder

    3. run build
        - in the visualization folder run:
        ./build/StockVisualizer
    
    # if it dont work good luck, i only use a arm apple and a intel windows.



# What needs to be done as of 7/12/25

1. expand from OHCLV data and prepare the feature engineering and visualzation module for higher level book data like from databento.

2. outlier detection of some sort / screener.... but i need to figure out what are good screeners and what is the bullshit noise.