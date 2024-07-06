# SVF-LoRaWAN
A Security Verification Framework for LoRaWAN Protocol (SVF-LoRaWAN).

Welcome to the official repository for the Security Verification Framework for LoRaWAN (SVF-LoRaWAN), an open-source tool designed to enhance the security analysis and verification of the LoRaWAN protocol. This framework is based on the research presented in the paper "A Security Verification Framework for the LoRaWAN Protocol with Application in the Manufacturing Industry" by Wenting Dong, Huibiao Zhu, Sini Chen, and Ning Ge.

## SVF-LoRaWAN offers the following features:

- CSP Modeling: Utilizes Communicating Sequential Processes (CSP) to accurately model the LoRaWAN protocol.
- Automated Model Checking: Integrates with the PAT (Process Analysis Toolkit) to automate the verification of security properties.
- Intruder Simulation: Simulates various attacker models to assess the protocol's robustness against security threats.
- Support for Multiple Versions: Compatible with LoRaWAN versions V1.0 and V1.1.
- Extensible Design: Allows for easy extension and customization by researchers and developers.

## Getting Started
### Prerequisites
Before you begin, ensure you have the following installed:
- .NET Core 3.1 or higher
- PAT model checker (available：https://pat.comp.nus.edu.sg/?page_id=2660)
### Installation
- Clone the repository:
git clone https://github.com/Doreen-Dong/SVF-LoRaWAN.git
- Launch PAT:
Open the installed PAT application.
- Load the .csp File:
In the PAT interface, locate and click on the “File” menu, then select “Open”.
Navigate to the folder containing the .csp file, select the file, and click “Open”.
