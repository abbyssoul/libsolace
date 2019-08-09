/*
*  Copyright 2019 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*******************************************************************************
 * libSolace: A range filtering iterator template
 *	@file		solace/range_view.hpp
 *	@brief		Util template for interator with filtering
 ******************************************************************************/
#pragma once
#ifndef SOLACE_RANGE_VIEW_HPP
#define SOLACE_RANGE_VIEW_HPP


namespace Solace {

/// Until the time there is an implementation of ranges shipped with c++ - this is a trivial substitute for a filter
template<typename T, typename I>
struct RangeView {
	using const_reference = T const&;
	using pred_t = bool(*)(const_reference);

	using Iterator = I;

	struct Iter {
		const_reference operator* () const noexcept { return *_current; }

		bool operator!= (Iter const& rhs) const noexcept { return (_current != rhs._current); }

		Iter& operator++ () noexcept {
			if (_current == _end )
				return *this;

			do {
				++_current;
			} while (_current != _end && !_pred(*_current));

			return *this;
		}

		Iterator _current;
		Iterator _end;
		pred_t  _pred;
	};

	Iter begin() const noexcept   { return {_start, _end, _pred}; }
	Iter end() const noexcept     { return {_end, _end, _pred}; }

	Iterator _start;
	Iterator _end;
	pred_t  _pred;
};

}  // namespace Solace
#endif  // SOLACE_RANGE_VIEW_HPP
