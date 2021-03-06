#ifndef RGBD_RF_RANDOM_FOREST_HH__
#define RGBD_RF_RANDOM_FOREST_HH__

#include <rdf/common.h>
#include <rdf/Image.h>
#include <rdf/PixelInfo.h>
#include <rdf/TrainData.h>
#include <rdf/Node.h>
#include <rdf/Offset.h>
#include <rdf/SplitCandidate.h>

namespace rdf {

class RandomForest;

enum pixelSet {
    LEFT  = 0,
    RIGHT = 1
};

/** \brief Random forest training parameters.
 *
 *  This structure is used to specify all the training parameters of the
 *  random forest.
 *  
 *  @param treeNum is the number of trees in the forest.
 *  @param labelNum is the number of labels in the classification
 *  images.
 *  @param imgNum is the total number of images for the training.
 *  @param imgDir is the path to the directory with the images.
 *  @param maxDepth is the maximum depth that can grow a tree.
 *  @param minSampleCount is the minimum number of exaples that can be
 *  splited in two leaf nodes.
 *  @param samplePixelNum is the number of pixels to be chosen randomly
 *  form a single image.
 *  @param trainImgNum is the number of images to be chosen randomly.
 *  @param offsetNum is the number of offsets to be generated for each
 *  node.
 *  @param thresholdNum is the number of threshodls to be generated for
 *  each node.
 *  @param offsetRange is the range of values that can be generated for
 *  the offsets.
 *  @param thresholdRange is the range of values that can be generated 
 *  for the thresholds.
 */
class  trainParams {
    public:
            int treeNum;
            int labelNum;
            int imgNum;
            string imgDir;
            int maxDepth; 
            int minSampleCount;
            int samplePixelNum;
            int trainImgNum; 
            int offsetNum; 
            int thresholdNum;
            NumRange offsetRange;
            NumRange thresholdRange;

            trainParams() {};
};

/**
 *  @struct SCParams
 *
 *  This structure is used for the function bestSplitCandidate to
 *  generate a Split Canditate to separate the train data examples.
 *
 */
struct SCParams {
    RandomForest* forest;
    NumRange trainDataRange;
};

void* findSplitThread (void* args);



/** \brief Random forest algorithm.
 */
class RandomForest {
    private:
      
        // Training parameters
        trainParams* tp;

        /* Images */
        ImagePool::Ptr image_pool;

        /* Train data */
        TrainData::Ptr td;

        /* Array of the trees of the forest */
        std::vector <Node *> trees;

        /** \brief Returns the information gain by splitting the training set by the 
         * specified SplitCandidate.
         *
         *  \param[in] phi The SplitCandidate to evaluate.
         *  \param[in] set_entropy The entropy of the set before the split.
         *  \param[in] range The range of the training data where to evaluate the split 
         *  candidate.
         *  \return The information gain of the SplitCandidate evaluated on the 
         *  specified range of the training data.
         *
         */
        float G(
            const SplitCandidate& phi,
            const float setEntropy,
            NumRange setRange
        );

        /** \brief Shannon Entropy function
         *
         *  \param[in] probabilities is a vector that contains the percentage of
         *  each label occurrence in a set.
         *
         *  \return the entroply associated with this percentages.
         */
        float H(const std::vector<float>& percentage);

        /** \brief Returns a normalized vector with the dirstribution of each 
         * type of label in the TrainData vector.
         *
         *  \param[in] begin First range index of the TrainData vector  
         *  \param[in] begin Second range index of the TrainData vector  
         *  \return A normalize vector with the distribution of each type of 
         *  label
         */
        std::vector<float> labelDistribution(const size_t begin, const size_t end);

        /**
         *  This function determines if the given node stays as a leaf
         *  node or must be spliten y the training.
         *
         *  @param pointer to the node.
         *  @param range of the set.
         *  @param depth of the node.
         *  @param minimum number of examples per tree
         *
         *  @return true if the node must be a leaf or false in other case.
         */
        bool testNode (Node **n, 
                       Node *currentNode,
                       NumRange range);

        /** \brief Traverse the tree up to the root to figure out the nodes 
         * depth.
         *  \param[in] n Pointer to the node.
         *  \return Depth of the node.
         */
        int getDepth (Node *n);
        
        /** \brief Calculates the feature function given the offsets and the
         *  pixel.
         * 
         *  \param[in] u first pixel offset.
         *  \param[in] v second pixel offset.
         *  \param[in] x Pixel coordinate of the pixel for the feature to be
         *  calculated.
         * 
         *  \return value of the calculated feature.
         */
        float calcFeature(
            const Offset& u, 
            const Offset& v,
            const PixelInfo& pi,
            Image *img
        );

        /** 
         * Sorts the training data array and returns the index
         * which splits the  array into left and right sets.
         *
         * @param range of the train set.
         * @param f feature that corresponds to the best split of the
         * training data
         *
         * @return index that defines the best split of the array.
         */
        int sortData (NumRange range, SplitCandidate f);
     
        /**
         *  classifyPixel
         *
         *  This function calculate the feature phi on the pixel x and
         *  depending on its threshold it will classify it as LEFT or
         *  RIGHT.
         *
         *  @param phi is te feacture to classify pixel.
         *  @param x is the pixel to be classifyed.
         *
         *  @return LEFT or RIGHT depending on the classification of the
         *  pixel x by the feature phi.
         */
        pixelSet classifyPixel(SplitCandidate phi, PixelInfo x, Image *img); 

         /**
         *  This function run the training of a single tree.
         *
         *  @param treeID of the tree to construct.
         *  @param maximum depth of the tree to construct.
         *  @param maximun number of pixels in a leaf node.
         *  @param number of pixels to select.
         *  @param number of training images.
         *  @param reference to the train data.
         */
        void train (int treeID);

        /**
         *  writeNodeToFile
         *
         *  This function write the information of a node to a file.
         *
         *  @param pointer to the node.
         *  @param file pointer to the file.
         */
        void writeNodeToFile (Node *currentNode, FILE *fp);

        /**
         *  writeTreeToFile
         *
         *  Write a trained tree to a file.
         * 
         *  @param treeID is the id of the trained tree to write into a
         *  file.
         *  @param fileName is the string containing the path to the
         *  output file.
         *
         */
        void writeTreeToFile(int treeID, string fileName);
        
        /**
         *  loadNodeFromFile
         *
         *  This function loads the information of a node.
         *
         *  @param pointer to the node.
         *  @param file pointer to the file.
         */
        void loadNodeFromFile (Node **currentNode,
                               Node **sideNode,
                               char nodeType,
                               int nodeID,
                               int side,
                               std::stack<Node*> *nStack,
                               FILE *fp);

        /**
         *  loadTreeToFile
         *
         *  load a trained tree from a file.
         * 
         *  @param fileName is the string containing the path to the file.
         */
        void loadTreeFromFile(const std::string& filename);

    public:

        /** \brief Constructor */
        RandomForest () : tp(nullptr) {}

        /**
         *  TODO: implement the destructor
         */
        virtual ~RandomForest () {}
 
        /**
         *  bestSplitThreadFun
         *
         *  This function runs the bestSplitCandidate in diferent
         *  threads. It divides the work between them by the macro
         *  THREADS_PER_NODE.
         *
         *  @param structure with the parameter to generate a split
         *  candidate.
         *
         *  @return The best split candidate generated
         */
        SplitCandidate bestSplitThreadFun(NumRange range);

        /**
         *  bestSplitCandidate
         *
         *  This function generate and return a feature that maximise
         *  the function G. This function create split candidates
         *  randomly and test them in the train data. The best feature
         *  generated will be return.
         *
         *  @param a structure with the information to generate a split
         *  candidate.
         *
         *  @return the best split candidate generated.
         */
        SplitCandidate bestSplitCandidate(SCParams& params);

        /**
         *  This fuction visit all the nodes in a tree specified and
         *  print its content.
         *
         *  @param id of the tree.
         */
        void traversal(int treeID);

        /**
         *  This function classify a pixel of a given image by the
         *  random forest.
         *
         *  @param pointer to the image.
         *  @param pixel to classofy
         *  @param probability of the classification.
         *  @return label of the classification
         */
        Label predict (Image* img, PixelInfo pixel, float& prob);

        /**
         *  This function start the training of the forest.
         *
         *  @param maximum depth of the tree.
         *  @param maximum number of examples to build a leaf node.
         *  @param number of pixels of an image for the training set.
         *  @param number of images per tree.
         *  @param number of offsets to generate.
         *  @param number of thresholds to generate.
         *
         */
        void trainForest(trainParams& tparams);

        /**
         *  Write the traided trees to diferent text files in a
         *  directory especified. The trees are saved in files named
         *  "i-Tree.txt".
         *
         *  @param path to the directory
         */
        void writeForest(const std::string& dirName);

        /**
         *  Load the trees contained in a directori with the name
         *  "i-Tree.txt".
         *
         *  @param path to the directory
         */
        void loadForest(int numTrees, int numLabels, const std::string& dirName);

        /**
         *  Return the percentage of classification of an image.
         *
         *  @param image to classify
         *  @return pecentage of pixels classified correctly
         */
        float testClassification (TrainImage& img);


        /**
         * Determines the percentage of pixels classified of an image and
         * print the classification to a image output file.
         * @param img Input image.
         * @return Percentage of pixels correctly classified.
         */
         //CHECK
        float testClassificationImage(TrainImage& img, const std::string& imgName);

};

} // namespace rdf

#endif // RGBD_RF_RANDOM_FOREST_HH__
