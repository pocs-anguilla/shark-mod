//===========================================================================
/*!
 * 
 *
 * \brief       Weighted data sets for (un-)supervised learning.
 * 
 * 
 * \par
 * This file provides containers for data used by the models, loss
 * functions, and learning algorithms (trainers). The reason for
 * dedicated containers of this type is that data often need to be
 * split into subsets, such as training and test data, or folds in
 * cross-validation. The containers in this file provide memory
 * efficient mechanisms for managing and providing such subsets.
 * The speciality of these containers are that they are weighted.
 * 
 * 
 *
 * \author    O. Krause
 * \date       2014
 *
 *
 * \par Copyright 1995-2017 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://shark-ml.org/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================

#ifndef SHARK_DATA_WEIGHTED_DATASET_H
#define SHARK_DATA_WEIGHTED_DATASET_H

#include <shark/Data/Dataset.h>
namespace shark {
	
///\brief Input-Label pair of data
template<class DataType, class WeightType>
struct WeightedDataPair{
	DataType data;
	WeightType weight;
	
	WeightedDataPair(){}
	
	template<class DataT, class WeightT>
	WeightedDataPair(
		DataT&& data,
		WeightT&& weight
	):data(data),weight(weight){}
	
	template<class DataT, class WeightT>
	WeightedDataPair(
		WeightedDataPair<DataT,WeightT> const& pair
	):data(pair.data),weight(pair.weight){}
	
	template<class DataT, class WeightT>
	WeightedDataPair& operator=(WeightedDataPair<DataT,WeightT> const& batch){
		data = batch.data;
		weight = batch.weight;
		return *this;
	}
	WeightedDataPair& operator=(WeightedDataPair const& batch){
		data = batch.data;
		weight = batch.weight;
		return *this;
	}
};

template<class D1, class W1, class D2, class W2>
void swap(WeightedDataPair<D1, W1>&& p1, WeightedDataPair<D2, W2>&& p2){
	using std::swap;
	swap(p1.data,p2.data);
	swap(p1.weight,p2.weight);
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<class DataType, class WeightType>
SHARK_CREATE_BATCH_INTERFACE(
	WeightedDataPair<DataType BOOST_PP_COMMA() WeightType>,
	(DataType, data)(WeightType, weight)
)
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace detail{
template <class DataContainerT>
class BaseWeightedDataset
{
public:
	typedef typename DataContainerT::element_type DataType;
	typedef double WeightType;
	typedef DataContainerT DataContainer;
	typedef Data<WeightType> WeightContainer;
	typedef typename DataContainer::IndexSet IndexSet;


	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef WeightedDataPair<DataType,WeightType> element_type;
	// TYPEDEFS FOR PAIRS
	typedef typename Batch<element_type>::shape_type shape_type;
	typedef typename Batch<element_type>::type value_type;
	typedef typename Batch<element_type>::proxy_type reference;
	typedef typename Batch<element_type>::const_proxy_type const_reference;


	///\brief Returns the number of batches of the set.
	std::size_t size() const{
		return m_data.size();
	}
	///\brief Returns the total number of elements.
	std::size_t numberOfElements() const{
		return m_data.numberOfElements();
	}

	///\brief Check whether the set is empty.
	bool empty() const{
		return m_data.empty();
	}

	///\brief Access to the stored data points as a separate container.
	DataContainer const& data() const{
		return m_data;
	}
	///\brief Access to the stored data points as a separate container.
	DataContainer& data(){
		return m_data;
	}

	///\brief Access to weights as a separate container.
	WeightContainer const& weights() const{
		return m_weights;
	}
	///\brief Access to weights as a separate container.
	WeightContainer& weights(){
		return m_weights;
	}
	
	///\brief Returns the shape of the elements in the dataset.
	shape_type shape() const{
		return {m_data.shape(), {}};
	}
	
	///\brief Set the shape of the elements in the dataset.
	void setShape(shape_type const& shape){
		m_data.setShape(shape.data);
	}
	///\brief Set the shape of the elements in the dataset.
	void setShape( typename DataContainer::shape_type const& shape){
		m_data.setShape(shape);
	}

	// CONSTRUCTORS

	///\brief Constructs an Empty data set.
	BaseWeightedDataset(){}

	/// \brief Construtor using a single element as blueprint to create a dataset with a specified number of elements.
	///
	/// Optionally the desired batch Size can be set
	///
	///@param size the new size of the container
	///@param element the blueprint element from which to create the Container
	///@param batchSize the size of the batches. if this is 0, the size is unlimited
	BaseWeightedDataset(std::size_t size, element_type const& element, std::size_t batchSize)
	: m_data(size,element.data,batchSize)
	, m_weights(size,element.weight,batchSize)
	{}

	///\brief Construction from data and a dataset rpresnting the weights
	///
	/// Beware that when calling this constructor the organization of batches must be equal in both
	/// containers. This Constructor will not reorganize the data!
	BaseWeightedDataset(DataContainer const& data, Data<WeightType> const& weights)
	: m_data(data), m_weights(weights)
	{
		SHARK_RUNTIME_CHECK(data.numberOfElements() == weights.numberOfElements(), "[ BaseWeightedDataset::WeightedData] number of data and number of weights must agree");
#ifndef DNDEBUG
		for(std::size_t i  = 0; i != data.size(); ++i){
			SIZE_CHECK(batchSize(data[i]) == batchSize(weights[i]));
		}
#endif
	}
	
	///\brief Construction from data. All points get the same weight assigned
	BaseWeightedDataset(DataContainer const& data, double weight)
	: m_data(data), m_weights(data.getPartitioning(), 1){
		for(std::size_t i = 0; i != size(); ++i){
			m_weights[i] = Batch<WeightType>::type(batchSize(m_data[i]),weight);
		}
	}

	// BATCH ACCESS
	reference operator[](std::size_t i){
		return {m_data[i],m_weights[i]};
	}
	const_reference operator[](std::size_t i) const{
		return {m_data[i],m_weights[i]};
	}
	
	// ITERATOR ACCESS
	typedef IndexingIterator<BaseWeightedDataset> iterator;
	typedef IndexingIterator<BaseWeightedDataset const> const_iterator;
	iterator begin(){
		return iterator(*this,0);
	}
	const_iterator begin()const{
		return const_iterator(*this,0);
	}
	iterator end(){
		return iterator(*this,size());
	}
	const_iterator end()const{
		return const_iterator(*this,size());
	}

	// MISC
	template<class Archive>
	void serialize(Archive & archive, unsigned int const){
		archive & m_data;
		archive & m_weights;
	}

	///\brief This method makes the vector independent of all siblings and parents.
	virtual void makeIndependent(){
		m_weights.makeIndependent();
		m_data.makeIndependent();
	}

	void splitBatch(std::size_t batch, std::size_t elementIndex){
		m_data.splitBatch(batch,elementIndex);
		m_weights.splitBatch(batch,elementIndex);
	}

	/// \brief Appends the contents of another data object to the end
	///
	/// The batches are not copied but now referenced from both datasets. Thus changing the appended
	/// dataset might change this one as well.
	void append(BaseWeightedDataset const& other){
		m_data.append(other.m_data);
		m_weights.append(other.m_weights);
	}
	
	/// \brief Creates a vector with the batch sizes of every batch.
	///
	/// This method can be used together to ensure
	/// that two datasets have the same batch structure.
	std::vector<std::size_t> getPartitioning()const{
		return m_data.getPartitioning();
	}

	friend void swap( BaseWeightedDataset& a, BaseWeightedDataset& b){
		swap(a.m_data,b.m_data);
		swap(a.m_weights,b.m_weights);
	}


	// SUBSETS

	///\brief Fill in the subset defined by the list of indices.
	BaseWeightedDataset indexedSubset(IndexSet const& indices) const{
		BaseWeightedDataset subset;
		subset.m_data = m_data.indexedSubset(indices);
		subset.m_weights = m_weights.indexedSubset(indices);
		return subset;
	}
private:
	DataContainer m_data;               /// point data
	WeightContainer m_weights; /// weight data
};
}

///
/// \brief Weighted data set for unsupervised learning
///
/// The WeightedData class extends Data for the
/// representation of data. In addition it holds and provides access to the corresponding weights.
///
/// WeightedData tries to mimic the underlying data as pairs of data points and weights.
/// this means that when accessing a batch by calling[i] or choosing one of the iterators
/// one access the input batch by[i].data and the weights by[i].weight
///
///this also holds true for single element access using operator(). Be aware, that direct access to element is
///a linear time operation. So it is not advisable to iterate over the elements, but instead iterate over the batches.
template <class DataT>
class WeightedData : public detail::BaseWeightedDataset <Data<DataT> >
{
private:
	typedef detail::BaseWeightedDataset <Data<DataT> > base_type;
public:
	using base_type::data;
	using base_type::weights;
	typedef typename base_type::DataType DataType;
	typedef typename base_type::WeightType WeightType;
	typedef typename base_type::element_type element_type;
	typedef DataT InputType;

	// CONSTRUCTORS

	///\brief Empty data set.
	WeightedData()
	{}

	///\brief Create an empty set with just the correct number of batches.
	///
	/// The user must initialize the dataset after that by himself.
	WeightedData(std::size_t numBatches)
	: base_type(numBatches)
	{}

	///\brief Construction from data.
	///
	/// Beware that when calling this constructor the organization of batches must be equal in both
	/// containers. This Constructor will not reorganize the data!
	WeightedData(Data<DataType> const& data, Data<WeightType> const& weights)
	: base_type(data,weights)
	{}
		
	///\brief Construction from data and a constant weight for all elements
	WeightedData(Data<DataType> const& data, double weight)
	: base_type(data,weight)
	{}
	
	///\brief Splits the container into two independent parts. The left part remains in the container, the right is stored as return type
	///
	///Order of elements remain unchanged. The SharedVector is not allowed to be shared for
	///this to work.
	WeightedData splice(std::size_t batch){
		return WeightedData(data().splice(batch),weights().splice(batch));
	}

	friend void swap(WeightedData& a, WeightedData& b){
		swap(static_cast<base_type&>(a),static_cast<base_type&>(b));
	}
};

/// specialized templates for generators returning labeled data batches
template<class I, class L>
using WeightedLabeledDataGenerator = Generator<WeightedDataPair<InputLabelPair<I,L>, double> >;

/// specialized templates for generators returning labeled data batches
template<class D, class L>
using WeightedGenerator = Generator<WeightedDataPair<D, double> >;

template<class D>
struct InputToDataType<WeightedDataPair<D, double > >{
	typedef WeightedData<D> type;
};

///brief  Outstream of elements for weighted data.
template<class T>
std::ostream &operator << (std::ostream &stream, const WeightedData<T>& d) {
	for(auto elem: elements(d))
		stream << elem.weight << " [" << elem.data<<"]"<< "\n";
	return stream;
}

/// \brief Returns a shuffled copy of the input data
///
/// The order of points is randomized and a copy of the initial data object returned.
/// The batch sizes are the same as in the original dataset.
/// \param data the dataset to shuffle
template<class T>
WeightedData<T> shuffle(WeightedData<T> const& data){
	return toDataset(randomSubset(elements(data), data.numberOfElements()),data.getPartitioning());
}

/// \brief creates a weighted unweighted data object from two ranges, representing data and weights
template<class DataRange, class WeightRange>
typename boost::disable_if<
	boost::is_arithmetic<WeightRange>,
	WeightedData<
		typename boost::range_value<DataRange>::type
	> 
>::type createDataFromRange(DataRange const& data, WeightRange const& weights, std::size_t batchSize = constants::DefaultBatchSize){

	SHARK_RUNTIME_CHECK(batchSize(data) == batchSize(weights),"Number of datapoints and number of weights must agree");

	typedef typename boost::range_value<DataRange>::type Data;

	return WeightedData<Data>(
		shark::createDataFromRange(data,batchSize),
		createDataFromRange(weights,batchSize)
	);
}


///
/// \brief Weighted data set for supervised learning
///
/// The WeightedLabeledData class extends LabeledData for the
/// representation of data. In addition it holds and provides access to the corresponding weights.
///
/// WeightedLabeledData tries to mimic the underlying data as pairs of data tuples(input,label) and weights.
/// this means that when accessing a batch by calling[i] or choosing one of the iterators
/// one access the databatch by[i].data and the weights by[i].weight. to access the points and labels
/// use[i].data.input and[i].data.label
///
///this also holds true for single element access using operator(). Be aware, that direct access to element is
///a linear time operation. So it is not advisable to iterate over the elements, but instead iterate over the batches.
///
/// It is possible to gains everal views on the set. one can either get access to inputs, labels and weights separately
/// or gain access to the unweighted dataset of inputs and labels. Additionally the sets support on-the-fly creation
/// of the (inputs,weights) subset for unsupervised weighted learning
template <class InputT, class LabelT>
class WeightedLabeledData : public detail::BaseWeightedDataset <LabeledData<InputT,LabelT> >
{
private:
	typedef detail::BaseWeightedDataset <LabeledData<InputT,LabelT> > base_type;
public:
	typedef typename base_type::DataType DataType;
	typedef typename base_type::WeightType WeightType;
	typedef InputT InputType;
	typedef LabelT LabelType;
	typedef typename base_type::element_type element_type;

	using base_type::data;
	using base_type::weights;

	// CONSTRUCTORS

	///\brief Empty data set.
	WeightedLabeledData(){}

	///\brief Create an empty set with just the correct number of batches.
	///
	/// The user must initialize the dataset after that by himself.
	WeightedLabeledData(std::size_t numBatches)
	: base_type(numBatches){}

	/// \brief Construtor using a single element as blueprint to create a dataset with a specified number of elements.
	///
	/// Optionally the desired batch Size can be set
	///
	///@param size the new size of the container
	///@param element the blueprint element from which to create the Container
	///@param batchSize the size of the batches. if this is 0, the size is unlimited
	WeightedLabeledData(std::size_t size, element_type const& element, std::size_t batchSize = constants::DefaultBatchSize)
	: base_type(size,element,batchSize){}

	///\brief Construction from data.
	///
	/// Beware that when calling this constructor the organization of batches must be equal in both
	/// containers. This Constructor will not reorganize the data!
	WeightedLabeledData(LabeledData<InputType,LabelType> const& data, Data<WeightType> const& weights)
	: base_type(data,weights)
	{}
		
	///\brief Construction from data and a constant weight for all elements
	WeightedLabeledData(LabeledData<InputType,LabelType> const& data, double weight)
	: base_type(data,weight)
	{}
		
	///\brief Access to the inputs as a separate container.
	Data<InputType> const& inputs() const{
		return data().inputs();
	}
	///\brief Access to the inputs as a separate container.
	Data<InputType>& inputs(){
		return data().inputs();
	}
	
	///\brief Access to the labels as a separate container.
	Data<LabelType> const& labels() const{
		return data().labels();
	}
	///\brief Access to the labels as a separate container.
	Data<LabelType>& labels(){
		return data().labels();
	}
	
	/// \brief Constructs an WeightedData object for the inputs.
	WeightedData<InputType> weightedInputs() const{
		return WeightedData<InputType>(data().inputs(),weights());
	}
	
	/// \brief Constructs an WeightedData object for the labels.
	WeightedData<LabelType> weightedLabels() const{
		return WeightedData<LabelType>(data().labels(),weights());
	}

	///\brief Splits the container into two independent parts. The left part remains in the container, the right is stored as return type
	///
	///Order of elements remain unchanged. The SharedVector is not allowed to be shared for
	///this to work.
	WeightedLabeledData splice(std::size_t batch){
		return WeightedLabeledData(data().splice(batch),weights().splice(batch));
	}

	friend void swap(WeightedLabeledData& a, WeightedLabeledData& b){
		swap(static_cast<base_type&>(a),static_cast<base_type&>(b));
	}
};

template<class I, class L>
struct InputToDataType<WeightedDataPair<InputLabelPair<I, L>, double > >{
	typedef WeightedLabeledData<I, L> type;
};

///brief  Outstream of elements for weighted labeled data.
template<class T, class U>
std::ostream &operator << (std::ostream &stream, const WeightedLabeledData<T, U>& d) {
	for(auto elem: elements(d))
		stream << elem.weight <<" ("<< elem.data.label << " [" << elem.data.input<<"] )"<< "\n";
	return stream;
}

/// \brief Returns a shuffled copy of the input data
///
/// The order of (input-label)-pairs is randomized and a copy of the initial data object returned.
/// The batch sizes are the same as in the original dataset.
/// \param data the dataset to shuffle
template<class I, class L>
WeightedLabeledData<I,L> shuffle(WeightedLabeledData<I,L> const& data){
	return toDataset(randomSubset(elements(data), data.numberOfElements()),data.getPartitioning());
}


//Stuff for Dimensionality and querying of basic information

inline std::size_t numberOfClasses(WeightedData<unsigned int> const& labels){
	return numberOfClasses(labels.data());
}

///\brief Returns the number of members of each class in the dataset.
inline std::vector<std::size_t> classSizes(WeightedData<unsigned int> const& labels){
	return classSizes(labels.data());
}

///\brief  Return the dimnsionality of points of a weighted dataset
template <class InputType>
std::size_t dataDimension(WeightedData<InputType> const& dataset){
	return dataDimension(dataset.data());
}

///\brief  Return the input dimensionality of a weighted labeled dataset.
template <class InputType, class LabelType>
std::size_t inputDimension(WeightedLabeledData<InputType, LabelType> const& dataset){
	return dataDimension(dataset.inputs());
}

///\brief  Return the label/output dimensionality of a labeled dataset.
template <class InputType, class LabelType>
std::size_t labelDimension(WeightedLabeledData<InputType, LabelType> const& dataset){
	return dataDimension(dataset.labels());
}
///\brief Return the number of classes (highest label value +1) of a classification dataset with unsigned int label encoding
template <class InputType>
std::size_t numberOfClasses(WeightedLabeledData<InputType, unsigned int> const& dataset){
	return numberOfClasses(dataset.labels());
}

///\brief Returns the number of members of each class in the dataset.
template<class InputType, class LabelType>
inline std::vector<std::size_t> classSizes(WeightedLabeledData<InputType, LabelType> const& dataset){
	return classSizes(dataset.labels());
}

///\brief Returns the total sum of weights.
template<class InputType>
double sumOfWeights(WeightedData<InputType> const& dataset){
	double weightSum = 0;
	for(std::size_t i = 0; i != dataset.size(); ++i){
		weightSum += sum(dataset[i].weight);
	}
	return weightSum;
}
///\brief Returns the total sum of weights.
template<class InputType, class LabelType>
double sumOfWeights(WeightedLabeledData<InputType,LabelType> const& dataset){
	double weightSum = 0;
	for(std::size_t i = 0; i != dataset.size(); ++i){
		weightSum += sum(dataset[i].weight);
	}
	return weightSum;
}

/// \brief Computes the cumulative weight of every class.
template<class InputType>
RealVector classWeight(WeightedLabeledData<InputType,unsigned int> const& dataset){
	RealVector weights(numberOfClasses(dataset),0.0);
	for(auto const& elem: elements(dataset)){
		weights(elem.data.label) += elem.weight;
	}
	return weights;
}

//creation of weighted datasets

/// \brief creates a weighted unweighted data object from two ranges, representing data and weights
template<class InputRange,class LabelRange, class WeightRange>
typename boost::disable_if<
	boost::is_arithmetic<WeightRange>,
	WeightedLabeledData<
		typename boost::range_value<InputRange>::type,
		typename boost::range_value<LabelRange>::type
	>
>::type createLabeledDataFromRange(InputRange const& inputs, LabelRange const& labels, WeightRange const& weights, std::size_t batchSize = constants::DefaultBatchSize){

	SHARK_RUNTIME_CHECK(batchSize(inputs) == batchSize(labels),
	"number of inputs and number of labels must agree");
	SHARK_RUNTIME_CHECK(batchSize(inputs) == batchSize(weights),
	"number of data points and number of weights must agree");
	typedef typename boost::range_value<InputRange>::type InputType;
	typedef typename boost::range_value<LabelRange>::type LabelType;

	return WeightedLabeledData<InputType,LabelType>(
		createLabeledDataFromRange(inputs,labels,batchSize),
		createDataFromRange(weights,batchSize)
	);
}

/// \brief Creates a bootstrap partition of a labeled dataset and returns it using weighting.
///
/// Bootstrapping resamples the dataset by drawing a set of points with
/// replacement. Thus the sampled set will contain some points multiple times
/// and some points not at all. Bootstrapping is usefull to obtain unbiased
/// measurements of the mean and variance of an estimator.
///
/// Optionally the size of the bootstrap (that is, the number of sampled points)
/// can be set. By default it is 0, which indicates that it is the same size as the original dataset.
template<class InputType, class LabelType>
WeightedLabeledData< InputType, LabelType> bootstrap(
	LabeledData<InputType,LabelType> const& dataset,
	std::size_t bootStrapSize = 0
){
	if(bootStrapSize == 0)
		bootStrapSize = dataset.numberOfElements();
	
	WeightedLabeledData<InputType,LabelType> bootstrapSet(dataset,0.0);

	auto booststrap =  elements(bootstrapSet);
	for(std::size_t i = 0; i != bootStrapSize; ++i){
		std::size_t index = random::discrete(random::globalRng(), std::size_t(0),bootStrapSize-1);
		booststrap[index].weight += 1.0;
	}
	bootstrapSet.setShape(dataset.shape());
	return bootstrapSet;
}

/// \brief Creates a bootstrap partition of an  dataset and returns it using weighting.
///
/// Bootstrapping resamples the dataset by drawing a set of points with
/// replacement. Thus the sampled set will contain some points multiple times
/// and some points not at all. Bootstrapping is usefull to obtain unbiased
/// measurements of the mean and variance of an estimator.
///
/// Optionally the size of the bootstrap (that is, the number of sampled points)
/// can be set. By default it is 0, which indicates that it is the same size as the original dataset.
template<class InputType>
WeightedData<InputType> bootstrap(
	Data<InputType> const& dataset,
	std::size_t bootStrapSize = 0
){
	if(bootStrapSize == 0)
		bootStrapSize = dataset.numberOfElements();
	
	WeightedData<InputType> bootstrapSet(dataset,0.0);

	auto booststrap =  elements(bootstrapSet);
	for(std::size_t i = 0; i != bootStrapSize; ++i){
		std::size_t index = random::discrete(random::globalRng(), std::size_t(0),bootStrapSize-1);
		booststrap[index].weight += 1.0;
	}
	bootstrapSet.setShape(dataset.shape());
	return bootstrapSet;
}

}

#endif
