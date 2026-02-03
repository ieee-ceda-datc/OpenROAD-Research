# OpenROAD-Research

OpenROAD-Research originates from [the OpenROAD Project](https://theopenroadproject.org/) and serves as a platform for developing and sharing next-generation physical design engines. It is designed to act as an open and collaborative innovation sandbox, complementing the production-quality [OpenROAD](https://github.com/The-OpenROAD-Project/OpenROAD) and [OpenROAD-flow-scripts](https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts) repositories.


## Missions
OpenROAD-Research and ORFS-Research are envisioned as foundational platforms to accelerate collaboration, preserve algorithmic diversity and drive innovation in physical design. Their core missions are to:

- **Serve as an open library of baselines and algorithms**: preserve diverse research contributions (e.g., multiple detailed placers with different heuristics), even if they are not yet production-ready.

- **Accelerate innovation**: lower the barrier for contributions in emerging areas, including LLM-based flow autotuning frameworks.

- **Support education and workforce development**: provide students and researchers with reproducible implementations and a broad collection of physical design algorithms.

- **Foster global collaboration**: act as “contrib” repositories (similar to PyTorch’s torch.contrib), where experimental and research-oriented code can coexist. OpenROAD-Research will host academically validated contributions, exploratory prototypes, and heterogeneous approaches.

## Research Directions
We anticipate significant contributions from the global EDA community, including:

- **GPU-accelerated P&R engines** integrated within OpenROAD-research, covering from partitioning to design rule checking (DRC), enabling substantial runtime improvements and scalability

- **ML-assisted physical design** where ML models are tightly integrated with OpenROAD-research to enable closed-loop optimization and QoR enhancement across the P&R flow

- **True 3D P&R engines** in OpenROAD-research that provide open, reproducible and transparent baselines for 3D integration

- **The pytorch in EDA** that offers modular operators and Python APIs within OpenROAD-research, to facilitate rapid prototyping and experimentation of ML-driven EDA methodologies


## How to Contribute
We welcome contributions from the community. To ensure consistency and maintain quality across the project, please follow the guidelines below:

- **Branching**: create your feature branch from the master branch.

- **Licensing**: all contributions should use a permissive open-source license, such as BSD 3-Clause License.

- **Testing**: verify that your changes do not break existing functionality and maintain expected behavior across benchmarks.

- **DCO Sign-Off**: each commit should be signed using the Developer Certificate of Origin (DCO). Use the -s flag when committing:

  ```bash
  git commit -s -m "Your commit message"
  ```

- **Submitting a Pull Request**: open a Pull Request (PR) targeting the main branch.

- **Reviewing**: upon submission, your PR will undergo automated regression testing. If the tests pass, your contribution will be accepted and merged. If any issues are identified, we will provide feedback to help you make the necessary revisions.

  
## Acknowledgments
Contributions to OpenROAD-Research and ORFS-Research are led by:
- **Prof. Zhiang Wang** (Fudan University),  contact email:  zhiangwang@fudan.edu.cn

  
We welcome contributions and collaborations from the broader community.


## License

BSD 3-Clause License. See [LICENSE](LICENSE) file.