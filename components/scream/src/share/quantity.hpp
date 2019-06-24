// Boost.Units - A C++ library for zero-overhead dimensional analysis and
// unit/quantity manipulation and conversion
//
// Copyright (C) 2003-2008 Matthias Christian Schabel
// Copyright (C) 2007-2008 Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef SCREAM_BOOST_UNITS_QUANTITY_HPP
#define SCREAM_BOOST_UNITS_QUANTITY_HPP

#include <boost/units/quantity.hpp>
#include "share/scream_types.hpp"


template<typename T>
struct Cazzo {};

namespace boost {

namespace units {


/// class declaration
template<class Unit>
class quantity<Unit,scream::Real>
{
        // base units are not the same as units.
        BOOST_MPL_ASSERT_NOT((detail::is_base_unit<Unit>));
        enum { force_instantiation_of_unit = sizeof(Unit) };
        typedef void (quantity::*unspecified_null_pointer_constant_type)(int*******);
    public:
        using Y = scream::Real;
        typedef quantity<Unit,Y>                        this_type;

        typedef Y                                       value_type;
        typedef Unit        unit_type;

        BOOST_CONSTEXPR quantity() : val_()
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        BOOST_CONSTEXPR quantity(const value_type& val) : val_(val)
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        BOOST_CONSTEXPR quantity(unspecified_null_pointer_constant_type) : val_()
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        BOOST_CONSTEXPR quantity(const this_type& source) : val_(source.val_)
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        // Need to make sure that the destructor of
        // Unit which contains the checking is instantiated,
        // on sun.
        #ifdef __SUNPRO_CC
        ~quantity() {
            unit_type force_unit_instantiation;
        }
        #endif

        //~quantity() { }

        BOOST_CXX14_CONSTEXPR this_type& operator=(const this_type& source)
        {
             val_ = source.val_;

             return *this;
        }

        #ifndef BOOST_NO_SFINAE

        /// implicit conversion between value types is allowed if allowed for value types themselves
        template<class YY>
        BOOST_CONSTEXPR quantity(const quantity<Unit,YY>& source,
            typename boost::enable_if<detail::is_non_narrowing_conversion<YY, Y> >::type* = 0) :
            val_(source.value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        /// implicit conversion between value types is not allowed if not allowed for value types themselves
        template<class YY>
        explicit BOOST_CONSTEXPR quantity(const quantity<Unit,YY>& source,
            typename boost::disable_if<detail::is_non_narrowing_conversion<YY, Y> >::type* = 0) :
            val_(static_cast<Y>(source.value()))
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        #else

        /// implicit conversion between value types is allowed if allowed for value types themselves
        template<class YY>
        BOOST_CONSTEXPR quantity(const quantity<Unit,YY>& source) :
            val_(source.value())
        {
            BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
            // BOOST_STATIC_ASSERT((boost::is_convertible<YY, Y>::value == true));
        }

        #endif

        /// implicit assignment between value types is allowed if allowed for value types themselves
        template<class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator=(const quantity<Unit,YY>& source)
        {
            BOOST_STATIC_ASSERT((boost::is_convertible<YY, Y>::value == true));

            *this = this_type(source);

            return *this;
        }

        #ifndef BOOST_NO_SFINAE

        /// explicit conversion between different unit systems is allowed if implicit conversion is disallowed
        template<class Unit2,class YY>
        explicit
        BOOST_CONSTEXPR quantity(const quantity<Unit2,YY>& source,
                 typename boost::disable_if<
                    mpl::and_<
                        //is_implicitly_convertible should be undefined when the
                        //units are not convertible at all
                        typename is_implicitly_convertible<Unit2,Unit>::type,
                        detail::is_non_narrowing_conversion<YY, Y>
                    >,
                    typename detail::disable_if_is_same<Unit, Unit2>::type
                 >::type* = 0)
             : val_(conversion_helper<quantity<Unit2,YY>,this_type>::convert(source).value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
            BOOST_STATIC_ASSERT((boost::is_convertible<YY,Y>::value == true));
        }

        /// implicit conversion between different unit systems is allowed if each fundamental dimension is implicitly convertible
        template<class Unit2,class YY>
        BOOST_CONSTEXPR quantity(const quantity<Unit2,YY>& source,
                 typename boost::enable_if<
                     mpl::and_<
                         typename is_implicitly_convertible<Unit2,Unit>::type,
                         detail::is_non_narrowing_conversion<YY, Y>
                     >,
                     typename detail::disable_if_is_same<Unit, Unit2>::type
                 >::type* = 0)
             : val_(conversion_helper<quantity<Unit2,YY>,this_type>::convert(source).value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
            BOOST_STATIC_ASSERT((boost::is_convertible<YY,Y>::value == true));
        }

        #else

        /// without SFINAE we can't distinguish between explicit and implicit conversions so
        /// the conversion is always explicit
        template<class Unit2,class YY>
        explicit BOOST_CONSTEXPR quantity(const quantity<Unit2,YY>& source)
             : val_(conversion_helper<quantity<Unit2,YY>,this_type>::convert(source).value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
            BOOST_STATIC_ASSERT((boost::is_convertible<YY,Y>::value == true));
        }

        #endif

        /// implicit assignment between different unit systems is allowed if each fundamental dimension is implicitly convertible
        template<class Unit2,class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator=(const quantity<Unit2,YY>& source)
        {
// typename Cazzo<Unit2>::grande i;
typename Cazzo<unit_type>::grande j;

            BOOST_STATIC_ASSERT((is_implicitly_convertible<Unit2,unit_type>::value == true));
            BOOST_STATIC_ASSERT((boost::is_convertible<YY,Y>::value == true));

            *this = this_type(source);

            return *this;
        }

        BOOST_CONSTEXPR const value_type& value() const     { return val_; }                        ///< constant accessor to value

        ///< can add a quantity of the same type if add_typeof_helper<value_type,value_type>::type is convertible to value_type
        template<class Unit2, class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator+=(const quantity<Unit2, YY>& source)
        {
            BOOST_STATIC_ASSERT((boost::is_same<typename add_typeof_helper<Unit, Unit2>::type, Unit>::value));
            val_ += source.value();
            return *this;
        }

        ///< can subtract a quantity of the same type if subtract_typeof_helper<value_type,value_type>::type is convertible to value_type
        template<class Unit2, class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator-=(const quantity<Unit2, YY>& source)
        {
            BOOST_STATIC_ASSERT((boost::is_same<typename subtract_typeof_helper<Unit, Unit2>::type, Unit>::value));
            val_ -= source.value();
            return *this;
        }

        template<class Unit2, class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator*=(const quantity<Unit2, YY>& source)
        {
            BOOST_STATIC_ASSERT((boost::is_same<typename multiply_typeof_helper<Unit, Unit2>::type, Unit>::value));
            val_ *= source.value();
            return *this;
        }

        template<class Unit2, class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator/=(const quantity<Unit2, YY>& source)
        {
            BOOST_STATIC_ASSERT((boost::is_same<typename divide_typeof_helper<Unit, Unit2>::type, Unit>::value));
            val_ /= source.value();
            return *this;
        }

        ///< can multiply a quantity by a scalar value_type if multiply_typeof_helper<value_type,value_type>::type is convertible to value_type
        BOOST_CXX14_CONSTEXPR this_type& operator*=(const value_type& source) { val_ *= source; return *this; }
        ///< can divide a quantity by a scalar value_type if divide_typeof_helper<value_type,value_type>::type is convertible to value_type
        BOOST_CXX14_CONSTEXPR this_type& operator/=(const value_type& source) { val_ /= source; return *this; }

        /// Construct quantity directly from @c value_type (potentially dangerous).
        static BOOST_CONSTEXPR this_type from_value(const value_type& val)  { return this_type(val, 0); }

    protected:
        explicit BOOST_CONSTEXPR quantity(const value_type& val, int) : val_(val) { }

    private:
        value_type    val_;
};

/// Specialization for dimensionless quantities. Implicit conversions between
/// unit systems are allowed because all dimensionless quantities are equivalent.
/// Implicit construction and assignment from and conversion to @c value_type is
/// also allowed.
template<class System>
class quantity<BOOST_UNITS_DIMENSIONLESS_UNIT(System),scream::Real>
{
    public:
        using Y = scream::Real;
        typedef quantity<unit<dimensionless_type,System>,Y>     this_type;

        typedef Y                                               value_type;
        typedef System                                          system_type;
        typedef dimensionless_type                              dimension_type;
        typedef unit<dimension_type,system_type>                unit_type;

        BOOST_CONSTEXPR quantity() : val_()
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        /// construction from raw @c value_type is allowed
        BOOST_CONSTEXPR quantity(value_type val) : val_(val)
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        BOOST_CONSTEXPR quantity(const this_type& source) : val_(source.val_)
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        //~quantity() { }

        BOOST_CXX14_CONSTEXPR this_type& operator=(const this_type& source)
        {
            val_ = source.val_;

            return *this;
        }

        #ifndef BOOST_NO_SFINAE

        /// implicit conversion between value types is allowed if allowed for value types themselves
        template<class YY>
        BOOST_CONSTEXPR quantity(const quantity<unit<dimension_type,system_type>,YY>& source,
            typename boost::enable_if<detail::is_non_narrowing_conversion<YY, Y> >::type* = 0) :
            val_(source.value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        /// implicit conversion between value types is not allowed if not allowed for value types themselves
        template<class YY>
        explicit BOOST_CONSTEXPR quantity(const quantity<unit<dimension_type,system_type>,YY>& source,
            typename boost::disable_if<detail::is_non_narrowing_conversion<YY, Y> >::type* = 0) :
            val_(static_cast<Y>(source.value()))
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        #else

        /// implicit conversion between value types is allowed if allowed for value types themselves
        template<class YY>
        BOOST_CONSTEXPR quantity(const quantity<unit<dimension_type,system_type>,YY>& source) :
            val_(source.value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
            BOOST_STATIC_ASSERT((boost::is_convertible<YY, Y>::value == true));
        }

        #endif

        /// implicit assignment between value types is allowed if allowed for value types themselves
        template<class YY>
        BOOST_CXX14_CONSTEXPR this_type& operator=(const quantity<unit<dimension_type,system_type>,YY>& source)
        {
            BOOST_STATIC_ASSERT((boost::is_convertible<YY,Y>::value == true));

            *this = this_type(source);

            return *this;
        }

        #if 1

        /// implicit conversion between different unit systems is allowed
        template<class System2, class Y2>
        BOOST_CONSTEXPR quantity(const quantity<unit<dimensionless_type, System2>,Y2>& source,
        #ifdef __SUNPRO_CC
            typename boost::enable_if<
                boost::mpl::and_<
                    detail::is_non_narrowing_conversion<Y2, Y>,
                    detail::is_dimensionless_system<System2>
                >
            >::type* = 0
        #else
            typename boost::enable_if<detail::is_non_narrowing_conversion<Y2, Y> >::type* = 0,
            typename detail::disable_if_is_same<System, System2>::type* = 0,
            typename boost::enable_if<detail::is_dimensionless_system<System2> >::type* = 0
        #endif
            ) :
            val_(source.value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        /// implicit conversion between different unit systems is allowed
        template<class System2, class Y2>
        explicit BOOST_CONSTEXPR quantity(const quantity<unit<dimensionless_type, System2>,Y2>& source,
        #ifdef __SUNPRO_CC
            typename boost::enable_if<
                boost::mpl::and_<
                    boost::mpl::not_<detail::is_non_narrowing_conversion<Y2, Y> >,
                    detail::is_dimensionless_system<System2>
                >
            >::type* = 0
        #else
            typename boost::disable_if<detail::is_non_narrowing_conversion<Y2, Y> >::type* = 0,
            typename detail::disable_if_is_same<System, System2>::type* = 0,
            typename boost::enable_if<detail::is_dimensionless_system<System2> >::type* = 0
        #endif
            ) :
            val_(static_cast<Y>(source.value()))
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        #else

        /// implicit conversion between different unit systems is allowed
        template<class System2, class Y2>
        BOOST_CONSTEXPR quantity(const quantity<unit<dimensionless_type,homogeneous_system<System2> >,Y2>& source) :
            val_(source.value())
        {
            BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
            BOOST_STATIC_ASSERT((boost::is_convertible<Y2, Y>::value == true));
        }

        #endif

        /// conversion between different unit systems is explicit when
        /// the units are not equivalent.
        template<class System2, class Y2>
        explicit BOOST_CONSTEXPR quantity(const quantity<unit<dimensionless_type, System2>,Y2>& source,
            typename boost::disable_if<detail::is_dimensionless_system<System2> >::type* = 0) :
            val_(conversion_helper<quantity<unit<dimensionless_type, System2>,Y2>, this_type>::convert(source).value())
        {
            // BOOST_UNITS_CHECK_LAYOUT_COMPATIBILITY(this_type, Y);
        }

        #ifndef __SUNPRO_CC

        /// implicit assignment between different unit systems is allowed
        template<class System2>
        BOOST_CXX14_CONSTEXPR this_type& operator=(const quantity<BOOST_UNITS_DIMENSIONLESS_UNIT(System2),Y>& source)
        {
            *this = this_type(source);

            return *this;
        }

        #endif

        /// implicit conversion to @c value_type is allowed
        BOOST_CONSTEXPR operator value_type() const                               { return val_; }

        BOOST_CONSTEXPR const value_type& value() const                           { return val_; }  ///< constant accessor to value

        ///< can add a quantity of the same type if add_typeof_helper<value_type,value_type>::type is convertible to value_type
        BOOST_CXX14_CONSTEXPR this_type& operator+=(const this_type& source)      { val_ += source.val_; return *this; }

        ///< can subtract a quantity of the same type if subtract_typeof_helper<value_type,value_type>::type is convertible to value_type
        BOOST_CXX14_CONSTEXPR this_type& operator-=(const this_type& source)      { val_ -= source.val_; return *this; }

        ///< can multiply a quantity by a scalar value_type if multiply_typeof_helper<value_type,value_type>::type is convertible to value_type
        BOOST_CXX14_CONSTEXPR this_type& operator*=(const value_type& val)        { val_ *= val; return *this; }

        ///< can divide a quantity by a scalar value_type if divide_typeof_helper<value_type,value_type>::type is convertible to value_type
        BOOST_CXX14_CONSTEXPR this_type& operator/=(const value_type& val)        { val_ /= val; return *this; }

        /// Construct quantity directly from @c value_type.
        static BOOST_CONSTEXPR this_type from_value(const value_type& val)        { return this_type(val); }

   private:
        value_type    val_;
};

} // namespace units

} // namespace boost

#endif // BOOST_UNITS_QUANTITY_HPP
