
#include <stdio.h>
#include <fstream>
#include <ios>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/config/AWSProfileConfigLoader.h>
#include <aws/rekognition/RekognitionClient.h>
#include <aws/rekognition/model/ListCollectionsRequest.h>
#include <aws/rekognition/model/ListCollectionsResult.h>
#include <aws/rekognition/model/CreateCollectionRequest.h>
#include <aws/rekognition/model/IndexFacesRequest.h>
#include <aws/rekognition/model/Image.h>
#include <aws/rekognition/model/SearchFacesByImageRequest.h>
#include <aws/rekognition/model/ListFacesRequest.h>
#include <aws/rekognition/model/ListFacesResult.h>
#include <aws/core/platform/Environment.h>

using namespace std;
using namespace Aws;
using namespace Aws::Config;
using namespace Aws::Utils;
using namespace Aws::Rekognition;
using namespace Aws::Rekognition::Model;

Vector<String> list_collections(RekognitionClient & client) {
	ListCollectionsRequest request;
	ListCollectionsOutcome collections =  client.ListCollections(request);

	return collections.GetResult().GetCollectionIds();
}

shared_ptr<vector<unsigned char>> load_file(const String & fileName) {
	ifstream file(fileName.c_str());

	long begin = file.tellg();
	file.seekg (0, ios::end);
	size_t fileSize = file.tellg() - begin;
	file.seekg (0, ios::beg);

	shared_ptr<vector<unsigned char>> data(new vector<unsigned char>(fileSize));

	file.read((char*)data->data(), fileSize);
	file.close();

	return data;
}

void createCollection(RekognitionClient & client, const String & collectionId) {
	CreateCollectionRequest request;
	request.SetCollectionId(collectionId);
	CreateCollectionOutcome result = client.CreateCollection(request);

	if(result.IsSuccess()) {
		printf("Successful.\n");
	} else {
		printf("Failed to create collection: %s\n", result.GetError().GetMessage().c_str());
	}
}

void enroll(RekognitionClient & client, const String & collectionId, const String & fileName) {
	printf("Enrolling %s\n", fileName.c_str());
	shared_ptr<vector<unsigned char>> data = load_file(fileName);

	Image image;
	ByteBuffer imageData(data->data(), data->size());
	image.SetBytes(imageData);

	IndexFacesRequest request;
	request.SetImage(image);
	request.SetCollectionId(collectionId);
	IndexFacesOutcome result = client.IndexFaces(request);

	if(result.IsSuccess()) {
		printf("Successful enrollment.\n");
	} else {
		printf("Failed to index: %s\n", result.GetError().GetMessage().c_str());
	}
}

void recognize(RekognitionClient & client, const String & collectionId, const String & fileName){
	shared_ptr<vector<unsigned char>> data = load_file(fileName);

	Image image;
	ByteBuffer imageData(data->data(), data->size());
	image.SetBytes(imageData);

	SearchFacesByImageRequest request;
	request.SetCollectionId(collectionId);
	request.SetImage(image);
	auto result = client.SearchFacesByImage(request);

	if(result.IsSuccess()) {
		auto faces = result.GetResult().GetFaceMatches();
		printf("Found faces: %lu\n", faces.size());
		for(FaceMatch faceMatch: faces) {
			Face face = faceMatch.GetFace();
			printf("Found face, confidence: %f, face Id: %s\n", face.GetConfidence(), face.GetFaceId().c_str());
		}
	} else  {
		printf("Failed to index: %s\n", result.GetError().GetMessage().c_str());
	}
}

void listFaces(RekognitionClient & client, const char * collectionId){
	ListFacesRequest request;
	request.SetCollectionId(collectionId);
	ListFacesOutcome response = client.ListFaces(request);
	if(response.IsSuccess()) {
		ListFacesResult faces = response.GetResult();
		for(auto face : faces.GetFaces()) {
			printf("Face: %s\n", face.GetFaceId().c_str());
		}
		if(faces.GetFaces().size() == 0) {
			printf("<No faces>\n");
		}
	} else {
		printf("Failed to list faces: %s\n", response.GetError().GetMessage().c_str());
	}
}

void listCollections(RekognitionClient & client) {
	auto collectionIds = list_collections(client);
	for(String collectionId : collectionIds) {
		printf("\nCollectionId: %s\n", collectionId.c_str());
		listFaces(client, collectionId.c_str());
	}
}

struct Arguments {
	enum {NONE, LIST, CREATE_COLLECTION, ENROLL, RECOGINIZE} cmd = NONE;
	String collectionId;
	String imageFile;

	Arguments(int argc, char *argv[]) {
		if(argc == 2 && !strcmp("list", argv[1])){
			cmd = LIST;
		} else if(argc == 3 &&!strcmp("create", argv[1])){
			cmd = CREATE_COLLECTION;
			collectionId = argv[2];
		} else if(argc == 4 &&!strcmp("enroll", argv[1])){
			cmd = ENROLL;
			collectionId = argv[2];
			imageFile = argv[3];
		} else if(argc == 4 &&!strcmp("recognize", argv[1])){
			cmd = RECOGINIZE;
			collectionId = argv[2];
			imageFile = argv[3];
		}
		if (cmd == NONE) {
			usageAndExit();
		}
	}

	void usageAndExit() {
		printf("\naws-face-rekognition list\n");
		printf("aws-face-rekognition create <collectionId>\n");
		printf("aws-face-rekognition enroll <collectionId> <image file, png/jpeg>\n");
		printf("aws-face-rekognition recognize <collectionId> <img file, png/jpeg>\n\n");
		exit(0);
	}
};

int main(int argc, char *argv[]) {
	Arguments arguments(argc, argv);

	SDKOptions options;
	options.loggingOptions.logLevel = Logging::LogLevel::Info;
	InitAPI(options);

	Client::ClientConfiguration config;
	config.scheme = Http::Scheme::HTTPS;

	// Read the region from the local aws configuration
	String configFile = Environment::GetEnv("HOME") + "/.aws/config";
	Aws::Config::AWSConfigFileProfileConfigLoader loader(configFile);
	loader.Load();
	auto profiles = loader.GetProfiles();
	if (!profiles["default"].GetRegion().empty()) {
		config.region = profiles["default"].GetRegion();
	}

	printf("Region: %s\n", config.region.c_str());
	RekognitionClient client(config);

	switch (arguments.cmd) {
	case Arguments::LIST:
		listCollections(client);
		break;
	case Arguments::CREATE_COLLECTION:
		createCollection(client, arguments.collectionId);
		break;
	case Arguments::ENROLL:
		enroll(client, arguments.collectionId, arguments.imageFile);
		break;
	case Arguments::RECOGINIZE:
		recognize(client, arguments.collectionId, arguments.imageFile);
		break;
	default:
		break;
	}

	ShutdownAPI(options);
}

