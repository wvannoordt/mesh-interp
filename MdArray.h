#ifndef CMF_MD_ARRAY_H
#define CMF_MD_ARRAY_H

#include <type_traits>
#include "CallError.h"

template <typename arType, const int arRank = 1> struct MdArray
{
	arType* data;
	int rank = arRank;
	int dims[arRank];
    int idxCoeff[arRank];
    size_t totalSize;
    
    MdArray(const MdArray& rhs)
    {
        data = rhs.data;
        rank = rhs.rank;
        totalSize = rhs.totalSize;
        for (int i = 0; i < arRank; i++)
        {
            dims[i] = rhs.dims[i];
            idxCoeff[i] = rhs.idxCoeff[i];
        }
    }
    
    MdArray& operator =(const MdArray& rhs)
    {
        data = rhs.data;
        rank = rhs.rank;
        totalSize = rhs.totalSize;
        for (int i = 0; i < arRank; i++)
        {
            dims[i] = rhs.dims[i];
            idxCoeff[i] = rhs.idxCoeff[i];
        }
        return *this;
    }
    
    template <typename arType2, const int arRank2 = 1> MdArray<arType2, arRank2> ReCast(int index)
    {
        //Expand this to reCast in rank as well?
        static_assert(arRank2==arRank, "Incorrect rank when performing ReCast");
        MdArray<arType2, arRank2> output;
        for (int i = 0; i < arRank2; i++) output.dims[i] = dims[i];
        output.dims[index] = (output.dims[index]*sizeof(arType))/sizeof(arType2);
        output.data = (arType2*)data;
        output.idxCoeff[0] = 1;
        for (int i = 1; i < arRank; i++)
        {
            output.idxCoeff[i] = output.idxCoeff[i-1]*output.dims[i-1];
        }
        return output;
    }

	void Ralloc(int lev)
	{
		totalSize = 0;
	}

	template <typename Ts> void Ralloc(int lev, Ts t)
	{
		dims[lev] = t;
        idxCoeff[0] = 1;
        for (int i = 1; i < arRank; i++)
        {
            idxCoeff[i] = idxCoeff[i-1]*dims[i-1];
        }
        totalSize = 1;
        for (int i = 0; i < arRank; i++)
        {
            totalSize *= dims[i];
        }
	}

	template <typename T, typename... Ts> void Ralloc(int lev, T t, Ts... ts)
	{
		static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Integral type required for dimension initialization.");
		dims[lev] = t;
		Ralloc(lev+1, ts...);
	}

	template <typename... Ts> MdArray(Ts... ts)
	{
		Ralloc(0, ts...);
	}

	template <typename... Ts> MdArray(arType* ptr, Ts... ts)
	{
        data = ptr;
		Ralloc(0, ts...);
	}
	
    template <typename T>  inline arType * idxC(int lev, T t)
    {
        return data+idxCoeff[lev]*t;
    }

    template <typename T, typename... Ts>  inline arType * idxC(int lev, T t, Ts... ts)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Integral type required for dimension initialization.");
        return idxC(lev+1, ts...) + t*idxCoeff[lev];
    }

    template <typename... Ts>  inline arType & operator () (Ts... ts)
    {
        static_assert(sizeof...(Ts)==arRank, "Incorrect rank in array index");
        return *(idxC(0, ts...));
    }
    
    template <typename T>  inline size_t offsetInternal(int lev, T t)
    {
        return idxCoeff[lev]*t;
    }

    template <typename T, typename... Ts>  inline size_t offsetInternal(int lev, T t, Ts... ts)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Integral type required for dimension initialization.");
        return offsetInternal(lev+1, ts...) + t*idxCoeff[lev];
    }

    template <typename... Ts>  inline size_t offset(Ts... ts)
    {
        static_assert(sizeof...(Ts)==arRank, "Incorrect rank in array index");
        return offsetInternal(0, ts...);
    }
};

template <typename arType, const int... dims> struct StaticArray
{
	private:
		template <const int depth, typename ...T> static  constexpr size_t sprodr(void) {return 1;}
		template <const int depth, const int T, const int... Tss> static  constexpr size_t sprodr(void) {return (depth==0)?T:T*sprodr<depth-1, Tss...>();}
		template <const int depth, const int... Tss> static  constexpr size_t sprod(void) {return (depth==0)?1:sprodr<depth-1, Tss...>();}
		template <const int depth, typename index>  inline size_t idxR(index i)
		{
			static_assert(std::is_integral<index>::value, "Integral value for index required.");
			return i*sprod<sizeof...(dims)-1, dims...>();
		}
		template <const int depth, typename index, typename... indices>  inline size_t idxR(index i, indices... is)
		{
			static_assert(std::is_integral<index>::value, "Integral value for index required.");
			return sprod<depth, dims...>()*i + idxR<depth+1>(is...);
		}
	public:
		arType data[StaticArray::sprod<sizeof...(dims), dims...>()];
		template <typename... indices>  inline arType& operator () (indices... is) {return data[idxR<0>(is...)];}
         StaticArray(void) {}
         StaticArray(const arType val)
        {
            for (size_t i = 0; i < StaticArray::sprod<sizeof...(dims), dims...>(); i++)
            {
                data[i] = val;
            }
        }
         StaticArray& operator = (const arType val)
        {
            for (size_t i = 0; i < StaticArray::sprod<sizeof...(dims), dims...>(); i++)
            {
                data[i] = val;
            }
            return *this;
        }
};

template <const int... vals> struct Dims
{
	template <const int depth, typename ...T> static  constexpr size_t prodr(void) {return 1;}
	template <const int depth, const int T, const int... Tss> static  constexpr size_t prodr(void) {return (depth==0)?T:T*prodr<depth-1, Tss...>();}
	template <const int depth, const int... Tss> static  constexpr size_t prod(void) {return (depth==0)?1:prodr<depth-1, Tss...>();}
	static constexpr  size_t size(void)
	{
		return prod<sizeof...(vals), vals...>();
	}
	template <const int depth, typename index>  static inline size_t idxR(index i)
	{
		static_assert(std::is_integral<index>::value, "Integral value for index required.");
		return i*prod<sizeof...(vals)-1, vals...>();
	}
	template <const int depth, typename index, typename... indices>  static inline size_t idxR(index i, indices... is)
	{
		static_assert(std::is_integral<index>::value, "Integral value for index required.");
		return prod<depth, vals...>()*i + idxR<depth+1>(is...);
	}
	template <typename... indices> static inline  size_t offset(indices... is)
	{
		return idxR<0, indices...>(is...);
	}
};
template <class BaseType, class ArrayDims, class BaseContainer=BaseType[ArrayDims::size()]> struct DataView
{
	BaseContainer data;
	 DataView(void){}
	template <class rhsType>  DataView(const rhsType rhs)
	{
		data = rhs;
	}
	template <typename... indices>  inline BaseType& operator () (indices... is)
	{
		return data[ArrayDims::offset(is...)];
	}
};

#endif