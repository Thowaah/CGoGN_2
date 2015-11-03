/*
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps
 * Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 * Web site: http://cgogn.unistra.fr/
 * Contact information: cgogn@unistra.fr
 *
 */

#ifndef __CORE_CONTAINER_CHUNK_ARRAY_FACTORY__
#define __CORE_CONTAINER_CHUNK_ARRAY_FACTORY__


#include "core/container/chunk_array.h"

#include <iostream>
#include <map>

namespace cgogn
{


template <unsigned int CHUNKSIZE>
class ChunkArrayFactory
{
public:
	static std::map<std::string, ChunkArrayGen<CHUNKSIZE>*> mapCA_;

public:
	/**
	 * @brief register a type
	 * @param keyType name of type
	 * @param obj a ptr on object (new ChunkArray<32,int> for example) ptr will be deleted by clean method
	 */
	static void registerCA(const std::string& keyType, ChunkArrayGen<CHUNKSIZE>* obj)
	{
		if(mapCA_.find(keyType) == mapCA_.end())
			mapCA_[keyType]=obj;
	}

	/**
	 * @brief create a ChunkArray from a typename
	 * @param keyType typename of type store in ChunkArray
	 * @return ptr on created ChunkArray
	 */
	static ChunkArrayGen<CHUNKSIZE>* create(const std::string& keyType)
	{
		ChunkArrayGen<CHUNKSIZE>* tmp=NULL;
		typename std::map<std::string, ChunkArrayGen<CHUNKSIZE>*>::const_iterator it = mapCA_.find(keyType);

		if(it != mapCA_.end())
		{
			tmp = (it->second)->clone();
		}
		else
			std::cerr << "type "<<keyType<< " not registred in ChunkArrayFactory" <<std::endl;

		return tmp;
	}

	/**
	 * @brief free allocated object stored in map
	 */
	static void clean()
	{
		for (auto it: mapCA_)
			delete it.second;
	}

};

template <unsigned int CHUNKSIZE>
std::map<std::string, ChunkArrayGen<CHUNKSIZE>*> ChunkArrayFactory<CHUNKSIZE>::mapCA_= std::map<std::string, ChunkArrayGen<CHUNKSIZE>*>();



} // namespace CGoGN

#endif