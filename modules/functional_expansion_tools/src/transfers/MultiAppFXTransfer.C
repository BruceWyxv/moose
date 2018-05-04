//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblemBase.h"
#include "Function.h"
#include "MultiApp.h"
#include "UserObject.h"

#include "FXExecutioner.h"
#include "MultiAppFXTransfer.h"

registerMooseObject("FunctionalExpansionToolsApp", MultiAppFXTransfer);

template <>
InputParameters
validParams<MultiAppFXTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();

  params.addClassDescription("Transfers coefficient arrays between objects that are derived from "
                             "MutableCoefficientsInterface; currently includes the following "
                             "types: FunctionSeries, FXBoundaryUserObject, and FXVolumeUserObject");

  params.addRequiredParam<std::string>(
      "this_app_object_name",
      "Name of the MutableCoefficientsInterface-derived object in this app (LocalApp).");

  params.addRequiredParam<std::string>(
      "multi_app_object_name",
      "Name of the MutableCoefficientsInterface-derived object in the MultiApp.");

  return params;
}

MultiAppFXTransfer::MultiAppFXTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _this_app_object_name(getParam<std::string>("this_app_object_name")),
    _multi_app_object_name(getParam<std::string>("multi_app_object_name")),
    getMultiAppObject(NULL),
    getSubAppObject(NULL)
{
}

void
MultiAppFXTransfer::initialSetup()
{
  // Search for the _this_app_object_name in the LocalApp
  getMultiAppObject =
      scanProblemBaseForObject(_multi_app->problemBase(), _this_app_object_name, "MultiApp");
  if (getMultiAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _this_app_object_name, "' in MultiApp");

  // Search for the _multi_app_object_name in each of the MultiApps
  for (std::size_t i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
    {
      if (i == 0) // First time through, assign without checking against previous values
        getSubAppObject = scanProblemBaseForObject(
            _multi_app->appProblemBase(i), _multi_app_object_name, _multi_app->name());
      else if (getSubAppObject != scanProblemBaseForObject(_multi_app->appProblemBase(i),
                                                           _multi_app_object_name,
                                                           _multi_app->name()))
        mooseError("The name '",
                   _multi_app_object_name,
                   "' is assigned to two different object types. Please modify your input file and "
                   "try again.");
    }
  if (getSubAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _multi_app_object_name, "' in SubApp");
}

MultiAppFXTransfer::GetProblemObject
MultiAppFXTransfer::scanProblemBaseForObject(FEProblemBase & base,
                                             const std::string & object_name,
                                             const std::string & app_name) const
{
  /*
   * For now we are only considering Executioners, Functions, and UserObjects, as they are the only
   * types currently implemented with MutableCoefficientsInterface. Others may be added later.
   *
   * Executioners:
   *   FXExecutioner
   *
   * Functions:
   *   FunctionSeries
   *
   * UserObjects:
   *   FXBoundaryUserObject (via FXBaseUserObject)
   *   FXVolumeUserObject (via FXBaseUserObject)
   */
  MutableCoefficientsInterface * interface;

  // Block for objects that depend on the MooseApp corresponding to 'base'
  {
    MooseApp & moose_app = base.getMooseApp();

    // Executioner
    Executioner * executioner = moose_app.getExecutioner();
    FXExecutioner * fx_executioner = dynamic_cast<FXExecutioner *>(executioner);

    // Check to see if the executioner is a subclass of MutableCoefficientsInterface
    if (fx_executioner)
    {
      if (fx_executioner->getTransferName().compare(object_name) == 0)
        return &MultiAppFXTransfer::getMutableCoefficientsExecutioner;
      else
        // Provide only a warning: there may be may possible MultiApps for coupling candidates and
        // we are looking for a specific one
        mooseWarning(
            "In MultiAppFXTransfer '",
            name(),
            "': Executioner '",
            fx_executioner->getTransferName(),
            "' in MultiApp '",
            app_name,
            "' does not match the Executioner '",
            object_name,
            "' that you provided.",
            " Please update the name if this is the excutioner to which you are trying to couple.");
    }
  }

  // Functions
  if (base.hasFunction(object_name))
  {
    Function & function = base.getFunction(object_name);
    interface = dynamic_cast<MutableCoefficientsInterface *>(&function);

    // Check to see if the function is a subclass of MutableCoefficientsInterface
    if (interface)
      return &MultiAppFXTransfer::getMutableCoefficientsFunction;
    else
      mooseError("In MultiAppFXTransfer '",
                 name(),
                 "': Function '",
                 object_name,
                 "' in MultiApp '",
                 app_name,
                 "' does not inherit from MutableCoefficientsInterface.",
                 " Please change the function type and try again.");
  }

  // UserObjects
  if (base.hasUserObject(object_name))
  {
    // Get the non-const qualified UserObject, otherwise we would use getUserObject()
    UserObject * user_object = base.getUserObjects().getActiveObject(object_name).get();
    interface = dynamic_cast<MutableCoefficientsInterface *>(user_object);

    // Check to see if the userObject is a subclass of MutableCoefficientsInterface
    if (interface)
      return &MultiAppFXTransfer::getMutableCoefficientsUserOject;
    else
      mooseError("In MultiAppFXTransfer '",
                 name(),
                 "': UserObject '",
                 object_name,
                 "' in MultiApp '",
                 app_name,
                 "' does not inherit from MutableCoefficientsInterface.",
                 " Please change the function type and try again.");
  }

  return NULL;
}

MutableCoefficientsInterface &
MultiAppFXTransfer::getMutableCoefficientsExecutioner(FEProblemBase & base,
                                                      const std::string & /*object_name*/,
                                                      THREAD_ID /*thread*/) const
{
  return dynamic_cast<MutableCoefficientsInterface &>(*(base.getMooseApp().getExecutioner()));
}

MutableCoefficientsInterface &
MultiAppFXTransfer::getMutableCoefficientsFunction(FEProblemBase & base,
                                                   const std::string & object_name,
                                                   THREAD_ID thread) const
{
  return dynamic_cast<MutableCoefficientsInterface &>(base.getFunction(object_name, thread));
}

MutableCoefficientsInterface &
MultiAppFXTransfer::getMutableCoefficientsUserOject(FEProblemBase & base,
                                                    const std::string & object_name,
                                                    THREAD_ID thread) const
{
  // Get the non-const qualified UserObject, otherwise we would use getUserObject()
  UserObject * user_object = base.getUserObjects().getActiveObject(object_name, thread).get();

  return dynamic_cast<MutableCoefficientsInterface &>(*user_object);
}

void
MultiAppFXTransfer::execute()
{
  _console << "Beginning MultiAppFXTransfer: " << name() << std::endl;

  switch (_direction)
  {
    // LocalApp -> MultiApp
    case TO_MULTIAPP:
    {
      // Get a reference to the object in the LocalApp
      const MutableCoefficientsInterface & from_object =
          (this->*getMultiAppObject)(_multi_app->problemBase(), _this_app_object_name, 0);

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
      {
        if (_multi_app->hasLocalApp(i))
          for (THREAD_ID t = 0; t < libMesh::n_threads(); ++t)
          {
            // Get a reference to the object in each MultiApp
            MutableCoefficientsInterface & to_object =
                (this->*getSubAppObject)(_multi_app->appProblemBase(i), _multi_app_object_name, t);

            if (to_object.isCompatibleWith(from_object))
              to_object.importCoefficients(from_object);
            else
              mooseError("'",
                         _multi_app_object_name,
                         "' is not compatible with '",
                         _this_app_object_name,
                         "'");
          }
      }
      break;
    }

    // MultiApp -> LocalApp
    case FROM_MULTIAPP:
    {
      /*
       * For now we will assume that the transfers are 1:1 and the coefficients are synchronized
       * among all instances, thus we only need to grab the set of coefficients from the first
       * SubApp.
       */
      if (_multi_app->hasLocalApp(0))
      {
        // Get a reference to the first thread object in the first MultiApp
        const MutableCoefficientsInterface & from_object =
            (this->*getSubAppObject)(_multi_app->appProblemBase(0), _multi_app_object_name, 0);

        for (THREAD_ID t = 0; t < libMesh::n_threads(); ++t)
        {
          // Get a reference to the object in each LocalApp instance
          MutableCoefficientsInterface & to_object =
              (this->*getMultiAppObject)(_multi_app->problemBase(), _this_app_object_name, t);

          if (to_object.isCompatibleWith(from_object))
            to_object.importCoefficients(from_object);
          else
            mooseError("'",
                       _multi_app_object_name,
                       "' is not compatible with '",
                       _this_app_object_name,
                       "'");
        }
      }
      break;
    }
  }

  _console << "Finished MultiAppFXTransfer: " << name() << std::endl;
}
