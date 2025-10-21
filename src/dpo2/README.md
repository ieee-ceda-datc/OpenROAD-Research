# GPU-DPO: GPU-Accelerated Detailed Placement Optimization
This repository contains the code for implementing detailed placement operators on GPU, with multi-height cell relocation support.

## GPU-DPO Code Structure
```
GPU-DPO/   
├── src/dpl 
│   ├── src                             # Kernel function operators
│   │   ├── graphics                    # Graphics utilities
│   │   ├── infrastructure              # Placement related objectives
│   │   ├── objective                   # GPU database construction
│   │   ├── optimization                # Kernel operator implementations
│   │   │   ├── detailed_global.cu      # Global swap implementation
│   │   │   ├── detailed_mis.cu         # MIS implementation
│   │   │   └── detailed_reorder.cu     # Local reordering implementation
│   │   └── util                        # Utility related functions
│   ├── test                            # Detailed placement tests
│   │   ├── aes-multi-height.tcl        # AES design test script
│   │   └── jpeg-multi-height.tcl       # JPEG design test script
└── README.md                           # Project documentation
```
