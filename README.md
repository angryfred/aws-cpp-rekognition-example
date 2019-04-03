
# AWS C++ rekognition example
This example demonstrates how to use AWS C++ SDK rekognition with focus on the face recognition.

Main features:
- List collections with related enrolled faces
- Create collections
- Enroll face to a collection given the face image file
- Recognize a face given the collection id and the face image file

## Build

### Pre requsite
1) The tool assumes that you have built and installed the AWS C++ SDK. Follow the link for instruction how to do it, <https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup.html>

2) We also assume that the AWS cli is installed and configured, for installation see <https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-install.html>. Configure AWS cli configure like:
 
```
aws configure
```

### Build the tool
If you have been able to build and install the sdk, then simply run make:

```
make
```

## Usage
_Note!_

All configurations and credentials are taken from you local AWS home. It's located in ~/.aws/.

### List all enrolled faces for all Collections
To view all Collections enter following command:

_Example_

```
./aws-face-rekognition list
```

### Create Collection
To create a Collection in AWS:

_Example_

```
./aws-face-rekognition create test-123
```

### Enroll face
The following command will enroll a given image into AWS rekognition service:

_Example_

```
./aws-face-rekognition enroll 80 face123.jpeg
```

### Recognize face
In order to recognize a face, pass the collection id and the face image like:

_Example_

```
./aws-face-rekognition recognize 80 face456.jpeg
```
