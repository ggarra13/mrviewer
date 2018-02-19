#ifndef fltk_WidgetAssociation_h
#define fltk_WidgetAssociation_h

#include <fltk/FL_API.h>

namespace fltk {

class FL_API Widget;
class FL_API AssociationType;

/*! \brief Class used by the foreach() functions.
 *
 * Base class for the association functor that is used in foreach(). If you want to supply
 * your specific actions to do with the associated data found by the foreach() functions
 * you need to derive from this class and provide a new handle function.
 */
class FL_API AssociationFunctor {
 public:
  /*!
   * For each
   * found association this function is called. If the function returns true the
   * loop is aborted and the data pointer for the current association is returned
   */
  virtual bool handle(const AssociationType&, const Widget*, void* data) = 0;
};

/*! \relates AssociationType
 * This function allows traversing all associations of a certain association type, a certain widget,
 * both, or none of the constraints.
 * For each found widget the handle function in the associaionFunctor class is called. If that
 * function returns true the scan is aborted and the data for the current widget is returned
 * A NULL pointer for the AssociationType or the Widget pointer means to call the functor for all
 * AssociationTypes and/or all Widgets.
 *
 * The function either returns the first associated data for which the functor returns true, or NULL.
 * See also Widget::foreach() and AssociationType::foreach().
 */
FL_API void*  foreach(const AssociationType*, const Widget*, AssociationFunctor&);

/*! \brief Base class for the association type.
 *
 * FLTK allows you to attach any kind of user data to a widget. This data is automatically freed when the
 * widget to which it is attached is destroyed. Internally an association table is used to connect the
 * widget pointer with the data pointer that is why all the functions concerned with this feature contain
 * "association" in their name. The advantage of this is that no space is taken on widgets that do not
 * contain the data (or that have the "default value"), and that the destructor code is not linked in
 * if the association is not used.
 *
 * To be able to associate data and to be able to have a customized way of freeing the data you need
 * to derive from this class and then create an instance of that class. With the pointer to that instance
 * the type of the data is identified.
 *
 * possible uses:
 * - assign key shortcuts to certain widgets
 * - assign a tooltip to some widgets
 * - assign a help-index to widgets
 * - assign a unique identifier to widgets to remote controlling
 * - assign additional layouting data for new container widgets
 * - assign data needed by typesafe callback mechanisms
 * - assign all kind of data not always required within a widget / each widget
 */
class FL_API AssociationType {

 public:
  /*! \brief This function is called when associated data is freed
   * This function must be proveded when creating a data specific subclass. The function
   * must do whatever is necessary to free associated data. Most of the time it will be a cast
   * to the right datatype and a delete
   */
  virtual void destroy(void* data) const = 0;

  /*! \brief Finds all data of this association type for a widget
   * This function just calls fltk::foreach(this, wg, fkt). If \a wg
   * is NULL this function will find all data for any widget.
   */
  void* foreach(const Widget* wg, AssociationFunctor& fkt) { return fltk::foreach(this, wg, fkt); }
};

}

#endif
