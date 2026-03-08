<h1 align="center">
  <em>IGNITE</em>
</h1>

<h3 align="center">
Inference Governed by Nested lazy Ignition for Thermal Efficiency
</h3>

<h4 align="center">
| Arm CPU | X86 CPU |
</h4>

<h4 align="center">

[![Paper](https://img.shields.io/badge/view-paper-blue)](#)
[![Android App](https://img.shields.io/badge/android-app-pink)](https://github.com/kjh2159/ChatBotApp/)
[![Actions Status](https://github.com/UbiquitousLearning/mllm/workflows/Tests/badge.svg)](https://github.com/kjh2159/mllm/actions)
</h4>

- Plain C/C++ implementation without dependencies
- Supported: ARM NEON, x86 AVX2, etc
- Various quantization schemes
- Advanced support for *IGNITE*
  - Phase-level DVFS control (CPU/RAM)
  - Phase-level pause injection
  - Layer-level pause injection


<!-- ### Contents
- [Android Demo](#android-demo)
- [Support models](#support-models)
- [Quick Start](#quick-start)
    - [Get the Code](#get-the-code)
    - [Check prerequisites](#check-prerequisites)
    - [Run with the CPU of Android](#run-with-the-cpu-of-android)
    - [Run with the CPU on Termux](#run-with-the-cpu-on-termux)
    - [Run for Linux](#run-for-linux)
- [Customization](#customization)
    - [Convert models](#convert-models)
    - [Convert vocabulary](#convert-vocabulary)
    - [Quantize models](#quantize-models)
- [Acknowledgments](#acknowledgments)
- [License](#license) -->

## 🚂 Support engines

### Announcement

**Now, mllm engine is only supported. We are preparing llama.cpp support for public version.**

## 🎟️ Supports
<details>
  <summary><strong>Language models</strong></summary>

| Model                                                                       | CPU <br> FP32 | CPU <br> INT4  | *IGNITE* |
|-----------------------------------------------------------------------------|---------------|----------------|----------|
| [LLaMA3.2 1B](https://github.com/meta-llama/llama3) | [✔️](https://huggingface.co/mllmTeam/llama-3.2-1b-mllm/tree/main)  | [✔️](https://huggingface.co/mllmTeam/llama-3.2-1b-mllm/tree/main)   | ⭕ |
| [LLaMA3.2 3B](https://github.com/meta-llama/llama3) | [✔️](https://huggingface.co/mllmTeam/llama-3.2-3b-mllm/tree/main)  | [✔️](https://huggingface.co/mllmTeam/llama-3.2-3b-mllm/tree/main) | ⭕ |
| [Qwen1.5 0.5B](https://github.com/QwenLM/Qwen) | [✔️](https://huggingface.co/mllmTeam/qwen-1.5-0.5b-mllm/tree/main)  | [✔️](https://huggingface.co/mllmTeam/qwen-1.5-0.5b-mllm/tree/main) | ⭕ |
| [Qwen1.5 1.8B](https://github.com/QwenLM/Qwen) | [✔️](https://huggingface.co/mllmTeam/qwen-1.5-1.8b-mllm/tree/main)  | [✔️](https://huggingface.co/mllmTeam/qwen-1.5-1.8b-mllm/tree/main) | ⭕ |
| [Qwen2.5 0.5B](https://github.com/QwenLM/Qwen3) | [✔️](https://huggingface.co/kjh2159/Qwen2.5-0.5B-MLLM/tree/main)  | [✔️](https://huggingface.co/kjh2159/Qwen2.5-0.5B-MLLM/tree/main) | ⭕ |
| [Qwen2.5 1.5B](https://github.com/QwenLM/Qwen3) | [✔️](https://huggingface.co/mllmTeam/qwen-2.5-1.5b-mllm/tree/main)  | [✔️](https://huggingface.co/mllmTeam/qwen-2.5-1.5b-mllm/tree/main) | ⭕ |
| [Qwen3 0.6B](https://github.com/QwenLM/Qwen3) | [✔️](https://huggingface.co/mllmTeam/qwen-3-0.6b-mllm/tree/main)  | [✔️](https://huggingface.co/mllmTeam/qwen-3-0.6b-mllm/tree/main) | ⭕ |
| [Qwen3 1.7B](https://github.com/QwenLM/Qwen3) | [✔️](https://huggingface.co/kjh2159/Qwen3-1.7B-MLLM/tree/main)  | [✔️](https://huggingface.co/kjh2159/Qwen3-1.7B-MLLM/tree/main) | ⭕ |

> For other models, please refer to the following two hugging face repositories (extension type is .mllm): [mllm](https://huggingface.co/mllmTeam/models) and [*IGNITE*](https://huggingface.co/kjh2159/models/). <br>
> For Qwen3 family, it supports seamless thinking mode, which is possible to toggle through the `enable` option.
</details>

<details>
  <summary><strong>Devices</strong></summary>

#### Tested

| Model            | Arch.  | Backend            | OS      | CPU DVFS | RAM DVFS |
|------------------|--------|--------------------|---------|----------|----------|
| Google Pixel9    | armv8  | Tensor G4          | Android |✅        |✅       |
| Galaxy S24       | armv8  | Exynos 2400        | Android |✅        |✅       |
| Galaxy S25       | armv8  | Snapdragon 8 Elite | Android |✅        |❌       |
| Galaxy Z Fold 7  | armv8  | Snapdragon 8 Elite | Android |❌        |❌       |
| Our workstation* | amd64  | Intel              | Linux   |-         |-       |

> \* Linux device (our workstation) is tested for only response generation not ignite. <br>
> ⚠️ **Galaxy S25** might not accept rooting depending on device firmware version (over Bit/SW Rev. vale 7) <br>
> ⚠️ **Z Fold 7** do not accept rooting. <br>
> ⚠️ Basically, the target mobile devices might not support RAM DVFS.

</details>

## 🚀 Quick Start

### 1. Get the code

```bash
git clone --recursive https://github.com/kjh2159/mllm ignite
cd ignite
```

### 2. Check prerequisites

Building mllm requires following tools:

- gcc(11.4+) / clang (11.0+)
- CMake >= 3.18
- Android NDK Toolchains >= 26


> Note that building OpenMP libs on macOS may fail due to Apple LLVM compiler, so we disable OpenMP on macOS by default, you may experience slower performance on macOS. Build mllm is more recommended on Linux.

### 3-1. Run with CPU on Android shell

*`NOTE:` This project requires to root an android phone. Also, depending on your phones, RAM DVFS may not be supported. Please first check if your phone is available for RAM DVFS*

#### a. *Build*

  ```bash
  export ANDROID_NDK=/path/to/your/ndk
  cd scripts
  ./build_android.sh
  ```

#### b. *Download model: Qwen3 1.7B*

Download the model from [here](https://huggingface.co/mllmTeam/qwen-1.5-0.5b-mllm/tree/main) and place the model file in the directory of `models`, or using the following instructions

```bash
mkdir ../models && cd ../models
# Download qwen-1.5-0.5b-q4_k.mllm
wget https://huggingface.co/kjh2159/Qwen3-1.7B-MLLM/resolve/main/qwen3-1.7b-q4_k.mllm?download=true -o qwen-3-1.7b-q4k.mllm
```

#### c. *Run remote on Android Phone*

```bash
cd ..
sh scripts-arm/run-setup.sh
sh scripts-arm/run-remote.sh
```

Result are as followed:

```
[Q] Which airport is located in Maine, Sacramento International Airport or Knox County Regional Airport?
[A] K Knox County Regional Airport is located in Knox County, Maine

===========================================
            Inference
-------------------------------------------
  Load time: 0.540094 s
  Prefilling speed: 75.0717 tokens/s
  Decoding speed: 26.0652 tokens/s
===========================================
```


### 3-2. Run with CPU on Termux

*`NOTE:` This project requires to root an android phone. Also, depending on your phones, RAM DVFS may not be supported. Please first check if your phone is available for RAM DVFS*

#### a. *Build*

```bash
cd scripts
sh build.sh
```

#### b. *Download Qwen3 1.7B*

Download the model from [here](https://huggingface.co/mllmTeam/qwen-1.5-0.5b-mllm/tree/main) and place the model file in the directory of `models`, or using the following instructions

```bash
mkdir ../models && cd ../models
# Download qwen-1.5-0.5b-q4_k.mllm
wget https://huggingface.co/kjh2159/Qwen3-1.7B-MLLM/resolve/main/qwen3-1.7b-q4_k.mllm?download=true -o qwen-3-1.7b-q4k.mllm
```

#### c. *Run on Android phone*

<!-- ```bash
sh scripts-termux/run-setup.sh
sh scripts-termux/run.sh

# or -->
```bash
cd ..
chmod +x scripts-termux/ignite-qwen.sh
su -c "sh scripts-termux/ignite-qwen.sh 10 10 6 6" # to control cpu core allocation

# or
# su -c "taskset f0 sh scripts-termux/ignite-qwen.sh 10 10 6 6" # to control cpu core allocation
```

## 🛰️ Utils

### Download supported models

```bash
mkdir -p models
python3 tools/py/downloader.py
```

### Perfetto

To collect data from perfeto in pure C++ code, you should build the library and enable the build option manually.
You can build the perfetto library by following the script below.

*[FILL HERE]*

## 🪐 Customization

### Convert models

You can convert a pytorch/safetensor model to mllm model by yourself.

```bash
cd tools/convertor
pip install -r ./requirements.txt

# for one file pytorch model
python converter.py --input_model=model.pth --output_model=model.mllm --type=torch

# for multi-file pytorch model
python converter.py --input_model=pytorch_model.bin.index.json --output_model=model.mllm --type=torch

# for one file safetensor model
python converter.py --input_model=model.bin --output_model=model.mllm --type=safetensor

# for multi-file safetensor model
python converter.py --input_model=model.safetensors.index.json --output_model=model.mllm --type=safetensor
``` 

### Convert vocabulary

You can convert vocabulary to mllm vocabulary as followed.

```bash
cd tools/convertor
python vocab.py --input_file=tokenizer.json --output_file=vocab.mllm --type=Unigram
```

### Quantize models

You can quantize mllm model to int4 model by yourself.
mllm only support two quantize modes: Q4_0 and Q4_K.

```bash
cd bin
./quantize model.mllm model_q4_k.mllm Q4_K
```

## 💫 Caution

Depending on mobile device types and the individual device, the prefill and decode speeds might fluctuate. These negative phenomena are observed in almost all on-device LLM engines.



## ✨ Acknowledgments

*IGNITE* project reuses the base kernels and implementation of [mllm](https://github.com/UbiquitousLearning/mllm).
Also, mllm reuses many low-level kernel implementation from [ggml](https://github.com/ggerganov/ggml) on ARM CPU.
It also utilizes [stb](https://github.com/nothings/stb) and [wenet](https://github.com/wenet-e2e/wenet) for
pre-processing images and audios.
mllm also has benefitted from following projects: [llama.cpp](https://github.com/ggerganov/llama.cpp)
and [MNN](https://github.com/alibaba/MNN).

## License

### Overall Project License

This project is licensed under the terms of the MIT License. Please see the [LICENSE](LICENSE) file in the root
directory for the full text of the MIT License.

### Apache 2.0 Licensed Components

Certain component([wenet](https://github.com/wenet-e2e/wenet)) of this project is licensed under the Apache License 2.0.
These component is clearly identified in their respective subdirectories along with a copy of the Apache License 2.0.
For the full text of the Apache License 2.0, please refer to the [LICENSE-APACHE](third_party/wenet_audio/LICENSE) file
located in the relevant subdirectories.


