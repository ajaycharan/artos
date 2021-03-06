#ifndef ARTOS_CAFFEFEATUREEXTRACTOR_H
#define ARTOS_CAFFEFEATUREEXTRACTOR_H

#include "FeatureExtractor.h"
#include <vector>
#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>

namespace ARTOS
{

/**
* Uses Caffe to extract image features from a specific layer of a given
* Convolutional Neural Network (CNN).
*
* **Parameters of this feature extractor:**
*     - *netFile* (`string`) - path to the protobuf file specifying the network structure.
*     - *weightsFile* (`string`) - path to the file with the pre-trained weights for the network.
*     - *meanFile* (`string`) - path to a mean image file which has to be subtracted from each
*       sample before propagating it through the network. This may be either a binaryproto file with a
*       mean image or a plain text file with 3 values, one for each channel.
*     - *scalesFile* (`string`) - path to a text file with the maximum value of each unscaled feature
*       channel, computed over several images in advance. If specified, the features extracted from the
*       CNN will be scaled to [-1,1].  
*       **Note:** This parameter must not be set before `layerName`.
*     - *pcaFile* (`string`) - path to a binary file which contains a mean feature vector `m` and a
*       matrix `A` used for dimensionality reduction. If specified, each feature cell `c` extracted from
*       the CNN will be transformed to `ĉ = A^T * (c - m)`, after scaling has been applied.
*       The binary file must start with two integers specifying the number of rows and columns of `A`,
*       respectively. Those are followed by the coefficients of `m` and `A` (in row-major order), stored
*       as floats.  
*       **Note:** This parameter must not be set before `layerName`.
*     - *layerName* (`string`) - the name of the layer in the network to extract features from.
*       Features from multiple layers may be concatened by specifying the names of the layers as
*       comma-separated list.
*     - *maxImgSize* (`int`) - maximum size of input images (may be limited to save time and memory).
*       0 means no limit.
*
* @see http://caffe.berkeleyvision.org/
*
* @author Bjoern Barz <bjoern.barz@uni-jena.de>
*/
class CaffeFeatureExtractor : public FeatureExtractor
{

public:

    /**
    * Constructs an empty CaffeFeatureExtractor which is not yet ready to be used.
    * The parameters "netFile" and "weightsFile" have to be set before use.
    */
    CaffeFeatureExtractor();
    
    /**
    * Constructs a CaffeFeatureExtractor for a given pre-trained network.
    *
    * @param[in] netFile Path to the protobuf file specifying the network structure.
    *
    * @param[in] weightsFile Path to the file with the pre-trained weights for the network.
    *
    * @param[in] meanFile Optionally, path to a mean image file which has to be subtracted from each
    * sample before propagating it through the network.
    *
    * @param[in] layerName The name of the layer in the network to extract features from. Multiple layer
    * names may be specified as comma-separated list. If an empty string is given, the last layer before
    * the first fully connected layer will be selected.
    */
    CaffeFeatureExtractor(const std::string & netFile, const std::string & weightsFile,
                          const std::string & meanFile = "", const std::string & layerName = "");

    virtual ~CaffeFeatureExtractor() {};

    /**
    * @return Returns the unique identifier of this kind of feature extractor. That type specifier
    * must consist of alphanumeric characters + dashes + underscores only and must begin with a letter.
    */
    virtual const char * type() const override { return "Caffe"; };
    
    /**
    * @return Human-readable name of this feature extractor.
    */
    virtual const char * name() const override;

    /**
    * @return Returns the number of features this feature extractor extracts from each cell.
    */
    virtual int numFeatures() const override;
    
    /**
    * @return Returns the size of the cells used by this feature extractor in x and y direction.
    */
    virtual Size cellSize() const override;
    
    /**
    * Specifies the size of the border along the x and y dimension of an image, which gets lost
    * during feature extraction. This may be due to unpadded convolutions, for instance.  
    * For example, a border of size (4, 2) would indicate, that only the region between (4, 2) and
    * (width - 5, height - 3) of an image would be transformed into features.
    *
    * @return Returns the size of the border along each image dimension in pixels.
    */
    virtual Size borderSize() const override;
    
    /**
    * @return Returns a Size struct with the maximum sizes for image in x and y direction which
    * can be processed by this feature extractor. If any dimension is 0, the size of the image
    * along that dimension does not need to be limited.
    */
    virtual Size maxImageSize() const override;
    
    /**
    * @return Returns true if it is safe to call extract() in parallel from multiple threads.
    */
    virtual bool supportsMultiThread() const override;
    
    /**
    * @return Returns true if it is considered reasonable to process feature extraction of multiple
    * scales of an image by patchworking them together, so that multiple scales are processed at
    * once on a single plane, which will have the size of the largest scale.
    * The patchworkPadding() reported by the feature extractor will be used as padding between images
    * on the same plane.
    */
    virtual bool patchworkProcessing() const override;
    
    /**
    * Specifies the amount of padding which should be added between images on the same plane when
    * features are extracted using patchworking (see patchworkProcessing()).
    *
    * @return Returns the amount of padding to add between two images.
    */
    virtual Size patchworkPadding() const override;
    
    /**
    * Converts a width and height given in cells to pixels.
    *
    * @param[in] cells The size given in cells.
    *
    * @return Returns the corresponding size in pixels.
    */
    virtual Size cellsToPixels(const Size & cells) const override;
    
    /**
    * Converts a width and height given in pixels to cells.
    *
    * @param[in] pixels The size given in pixels.
    *
    * @return Returns the corresponding size in cells.
    */
    virtual Size pixelsToCells(const Size & pixels) const override;
    
    /**
    * Computes features for a given image.
    *
    * @param[in] img The image to compute features for.
    *
    * @param[out] feat Destination matrix to store the extracted features in.
    * It will be resized to fit the number of cells in the given image.
    *
    * @note This function must be thread-safe.
    */
    virtual void extract(const JPEGImage & img, FeatureMatrix & feat) const override;
    
    /**
    * Changes the value of an integer parameter specific to the concrete feature extraction method.
    *
    * @param[in] paramName The name of the parameter to be set.
    *
    * @param[in] val The new value for the parameter.
    *
    * @throws UnknownParameterException There is no string parameter with the given name.
    *
    * @throws std::invalid_argument The given value is not allowed for the given parameter.
    */
    virtual void setParam(const std::string & paramName, int32_t val) override;
    
    /**
    * Changes the value of a string parameter specific to this algorithm.
    *
    * @param[in] paramName The name of the parameter to be set.
    *
    * @param[in] val The new value for the parameter.
    *
    * @throws UnknownParameterException There is no string parameter with the given name.
    *
    * @throws std::invalid_argument The given value is not allowed for the given parameter.
    */
    virtual void setParam(const std::string & paramName, const std::string & val) override;


protected:

    std::shared_ptr< caffe::Net<float> > m_net; /**< The network. */
    cv::Scalar m_mean; /**< Image mean. */
    FeatureCell m_scales; /**< Maxima of each unscaled feature channel, computed over several images in advance. */
    FeatureCell m_pcaMean; /**< Mean of features extracted from the CNN, after scaling. Used for PCA. */
    ScalarMatrix m_pcaTransform; /**< Matrix used for dimensionality reduction. */
    int m_lastLayer; /**< Index of the last convolutional layer in the network before the fully connected network. */
    int m_numChannels; /**< Number of input channels of the network. */
    int m_numOutputChannels; /**< Sum of the number of channels of all layers to extract features from. */
    std::vector<int> m_layerIndices; /**< Indices of the layers to extract features from, in the order they appear in the net. */
    std::vector<Size> m_cellSize; /**< Cell size derived from the network structure for each layer to extract features from, relative to the previous layer. */
    std::vector<Size> m_borderSize; /**< Border size derived from the network structure for each layer to extract features from, relative to the previous layer. */
    
    enum class LayerType { OTHER, CONV, POOL };
    
    struct LayerParams
    {
        LayerType layerType;
        Size kernelSize;
        Size padding;
        Size stride;
    };
    
    /**
    * Tries to load the network using the current parameters of this feature extractor.
    * Nothing will be done if the parameters are not yet set up.
    *
    * @throws std::invalid_argument All required parameters have been set up, but the network could not be loaded.
    */
    virtual void loadNetwork();
    
    /**
    * Tries to load the image mean from the file specified in the parameter "meanFile".
    *
    * @throws std::invalid_argument The mean file could not be loaded.
    */
    virtual void loadMean();

    /**
    * Tries to load the maxima of each unscaled feature channel from the file
    * specified in the parameter "meanFile".
    *
    * @throws std::invalid_argument The mean file could not be loaded.
    */
    virtual void loadScales();
    
    /**
    * Tries to load the mean feature vector and the transformation matrix for dimensionality reduction from the file
    * specified in the parameter "pcaFile".
    *
    * @throws std::invalid_argument The PCA file could not be loaded.
    */
    virtual void loadPCAParams();
    
    /**
    * Caches information about the layer(s) specified in the parameter "layerName" if the net has already been loaded
    * and sets m_layerIndices, m_cellSize and m_borderSize accordingly.
    *
    * If none of the specified layers could be found, the last convolutional layer in the network will be used.
    */
    virtual void loadLayerInfo();
    
    /**
    * Retrieves some relevant parameters of a given layer.
    *
    * @param[in] layerIndex The index of the layer.
    *
    * @param[out] params A LayerParams struct which will retrieve the parameters of the layer.
    * If any parameter is missing for the given layer or the layer is neither a convolutional or pooling layer,
    * the respective parameters will be set to their default values.
    */
    virtual void getLayerParams(int layerIndex, LayerParams & params) const;
    
    /**
    * Wraps a cv::Mat object around each channel of the input layer and adds those wrappers to @p input_channels.
    */
    void wrapInputLayers(std::vector<cv::Mat> & input_channels) const;
    
    /**
    * Preprocesses a given image and stores the result in given channels of the input layer.
    *
    * @param[in] img The image.
    *
    * @param[out] input_channels Vector of cv::Mat objects which will retrieve the data of each channel
    * of the preprocessed image.
    */
    virtual void preprocess(const JPEGImage & img, std::vector<cv::Mat> & input_channels) const;
    
    /**
    * Cache of networks which have already been loaded to be used by different feature extractor instances.
    * Their key is a pair of the protobuf filename and the weights filename.
    */
    static std::map< std::pair<std::string, std::string>, std::weak_ptr< caffe::Net<float> > > netPool;


private:

    static bool initializedCaffe;

};

}

#endif
