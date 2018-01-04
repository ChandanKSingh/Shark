//===========================================================================
/*!
 * 
 *
 * \brief       Implements the Mean Model that can be used for ensemble classifiers
 * 
 * 
 *
 * \author      Kang Li, O. Krause
 * \date        2014
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

#ifndef SHARK_MODELS_MEANMODEL_H
#define SHARK_MODELS_MEANMODEL_H

namespace shark {
/// \brief Calculates the weighted mean of a set of models
template<class ModelType>
class MeanModel : public AbstractModel<typename ModelType::InputType, RealVector, typename ModelType::ParameterVectorType>
{
private:
	template<class T> struct tag{};

	template<class InputBatch>
	void doEval(InputBatch const& patterns, RealMatrix& outputs, tag<RealVector>)const{
		for(std::size_t i = 0; i != m_models.size(); i++) 
			noalias(outputs) += m_weight[i] * m_models[i](patterns);
		outputs /= m_weightSum;
	}
	template<class InputBatch>
	void doEval(InputBatch const& patterns, RealMatrix& outputs, tag<unsigned int>)const{
		blas::vector<unsigned int> responses;
		for(std::size_t i = 0; i != m_models.size(); ++i){
			m_models[i].eval(patterns, responses);
			for(std::size_t p = 0; p != patterns.size1(); ++p){
				SIZE_CHECK(responses(p) < m_outputDim);
				outputs(p,responses(p)) += m_weight[i];
			}
		}
		outputs /= m_weightSum;
	}
	typedef AbstractModel<typename ModelType::InputType, RealVector, typename ModelType::ParameterVectorType> ModelBaseType;
public:
	

	typedef typename ModelBaseType::BatchInputType BatchInputType;
	typedef typename ModelBaseType::BatchOutputType BatchOutputType;
	typedef typename ModelBaseType::ParameterVectorType ParameterVectorType;
	/// Constructor
	MeanModel():m_weightSum(0){}
	
	std::string name() const
	{ return "MeanModel"; }
	
	///\brief Returns the expected shape of the input
	Shape inputShape() const{
		return m_models.empty() ? Shape(): m_models.front().inputShape();
	}
	///\brief Returns the shape of the output
	Shape outputShape() const{
		return m_models.empty() ? Shape(): m_models.front().outputShape();
	}

	using ModelBaseType::eval;
	void eval(BatchInputType const& patterns, BatchOutputType& outputs)const{
		outputs.resize(patterns.size1(), m_outputDim);
		outputs.clear();
		doEval(patterns,outputs, tag<typename ModelType::OutputType>());
	}
	
	void eval(BatchInputType const& patterns, BatchOutputType& outputs, State& state)const{
		eval(patterns,outputs);
	}
	
	std::size_t outputSize() const{
		return m_outputDim;
	}


	/// This model does not have any parameters.
	ParameterVectorType parameterVector() const {
		return {};
	}

	/// This model does not have any parameters
	void setParameterVector(ParameterVectorType const& param) {
		SHARK_ASSERT(param.size() == 0);
	}
	void read(InArchive& archive){
		archive >> m_models;
		archive >> m_weight;
		archive >> m_weightSum;
		archive >> m_outputDim;
	}
	void write(OutArchive& archive)const{
		archive << m_models;
		archive << m_weight;
		archive << m_weightSum;
		archive << m_outputDim;
	}

	/// \brief Removes all models from the ensemble
	void clearModels(){
		m_models.clear();
		m_weight.clear();
		m_weightSum = 0.0;
	}

	/// \brief Adds a new model to the ensemble.
	///
	/// \param model the new model
	/// \param weight weight of the model. must be > 0
	void addModel(ModelType const& model, double weight = 1.0){
		SHARK_RUNTIME_CHECK(weight > 0, "Weights must be positive");
		m_models.push_back(model);
		m_weight.push_back(weight);
		m_weightSum += weight;
	}
	
	ModelType const& getModel(std::size_t index)const{
		return m_models[index];
	}

	/// \brief Returns the weight of the i-th model
	double const& weight(std::size_t i)const{
		return m_weight[i];
	}
	
	/// \brief sets the weight of the i-th model
	void setWeight(std::size_t i, double newWeight){
		m_weightSum += newWeight - m_weight[i];
		m_weight[i] = newWeight;
	}
	
	///\brief sets the dimensionality of the output
	void setOutputSize(std::size_t dim){
		m_outputDim = dim;
	}
	
	/// \brief Returns the number of models.
	std::size_t numberOfModels()const{
		return m_models.size();
	}

protected:
	/// \brief collection of models.
	std::vector<ModelType> m_models;

	/// \brief Weight of the mean.
	RealVector m_weight;

	/// \brief Total sum of weights.
	double m_weightSum;

	///\brief output dimensionality
	std::size_t m_outputDim;
};


}
#endif // SHARK_MODELS_MEANMODEL_H
