//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXInputParameters.h"

template <>
InputParameters
validParams<FXInputParameters>()
{
  InputParameters params = emptyInputParameters();

  params.addClassDescription("This parameters-oriented class is designed to work as a general "
                             "interface for specifying the basic functional expansion inputs via "
                             "the input file.");

  // The available composite series types.
  //   Cartesian:      1D, 2D, or 3D, depending on which of x, y, and z are present
  //   CylindricalDuo: planar disc expansion and axial expansion
  MooseEnum series_types("Cartesian CylindricalDuo");
  MooseEnum single_series_types_1D("Legendre");
  MooseEnum single_series_types_2D("Zernike");

  params.addRequiredParam<MooseEnum>(
      "series_type", series_types, "The type of function series to construct.");

  /*
   * This needs to use `unsigned int` instead of `std::size_t` because otherwise MOOSE errors at
   * runtime
   */
  params.addRequiredParam<std::vector<unsigned int>>("orders",
                                                     "The order of each series. These must be "
                                                     "defined as \"x y z\" for Cartesian, and \"z "
                                                     "disc\" for CylindricalDuo.");

  params.addParam<std::vector<Real>>("physical_bounds",
                                     "The physical bounds of the function series. These must be "
                                     "defined as \"x_min x_max y_min y_max z_min z_max\" for "
                                     "Cartesian, and \"axial_min axial_max disc_center1 "
                                     "disc_center2 radius\" for CylindricalDuo");

  params.addParam<MooseEnum>("x", single_series_types_1D, "The series to use for the x-direction.");
  params.addParam<MooseEnum>("y", single_series_types_1D, "The series to use for the y-direction.");
  params.addParam<MooseEnum>("z", single_series_types_1D, "The series to use for the z-direction.");

  params.addParam<MooseEnum>("disc",
                             single_series_types_2D,
                             "The series to use for the disc. Its direction is determined by "
                             "orthogonality to the declared direction of the axis.");

  return params;
}

FXInputParameters::FXInputParameters(const InputParameters & parameters)
  : _orders(convertOrders(parameters.get<std::vector<unsigned int>>("orders"))),
    _physical_bounds(parameters.get<std::vector<Real>>("physical_bounds")),
    _series_type_name(parameters.get<MooseEnum>("series_type")),
    _x(parameters.get<MooseEnum>("x")),
    _y(parameters.get<MooseEnum>("y")),
    _z(parameters.get<MooseEnum>("z")),
    _disc(parameters.get<MooseEnum>("disc"))
{
}

std::vector<std::size_t>
FXInputParameters::generateCharacteristics() const
{
  std::vector<std::size_t> characteristics;

  // Add the integer representation of the MooseEnum for the series represetation
  characteristics.push_back(_series_type_name);

  if (_series_type_name == "Cartesian")
  {
    // X axis
    if (_x.isValid())
    {
      characteristics.push_back(_x);
      characteristics.push_back(_orders[0]);
    }

    // Y axis
    if (_y.isValid())
    {
      characteristics.push_back(_y);
      characteristics.push_back(_orders[1]);
    }

    // Z zxis
    if (_z.isValid())
    {
      characteristics.push_back(_z);
      characteristics.push_back(_orders[2]);
    }
  }
  else if (_series_type_name == "CylindricalDuo")
  {
    // Axial orientation
    if (_x.isValid())
    {
      characteristics.push_back(_x);
    }
    else if (_y.isValid())
    {
      characteristics.push_back(_y);
    }
    else if (_z.isValid())
    {
      characteristics.push_back(_z);
    }
    characteristics.push_back(_orders[0]);

    // Disc orientation
    characteristics.push_back(_disc);
    characteristics.push_back(_orders[1]);
  }

  return characteristics;
}

std::vector<std::size_t>
FXInputParameters::convertOrders(const std::vector<unsigned int> & orders)
{
  return std::vector<std::size_t>(orders.begin(), orders.end());
}
