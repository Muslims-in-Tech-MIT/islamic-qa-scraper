# Scraper Project

## Overview
This project is a simple C++ scraper that uses `libcurl` for making HTTP requests and `libxml2` for parsing HTML content. It extracts questions, answers, and source links from a specified website and outputs them as a JSON file.

## Prerequisites
- Docker

## Quick Start
1. Clone the repository: 
   - git clone https://github.com/basilysf1709/qa-scraper.git
   - cd qa-scraper
2. Build the Docker image:
docker build -t scraper .
3. Run the application in a Docker container:
docker run --rm scraper

## Detailed Instructions

### Building the Docker Image
To build the Docker image, navigate to the directory containing the `Dockerfile` and run the following command:
docker build -t scraper .

This command builds a Docker image named `scraper` based on the instructions in the `Dockerfile`. The `.` at the end of the command denotes the current directory as the build context.

### Running the Scraper
To run the scraper, use the Docker command:
docker run --rm scraper

This command runs the scraper inside a new Docker container based on the `scraper` image. The `--rm` flag cleans up the container after the application exits.

### Output
The scraper outputs a JSON file named `maliki_questions.json`, which contains the scraped data. This file will be created in the working directory inside the Docker container.

### Customization
If you need to customize the application, you can modify the `main.cpp` file with your changes. Rebuild the Docker image after making your changes to ensure they are included in the Docker container.

## Dependencies
- `libcurl4-openssl-dev`: Used for making HTTP requests.
- `libxml2-dev`: Used for parsing HTML content.

## Contact
For any further questions regarding this project, please contact basilyusuf1709@gmail.com
