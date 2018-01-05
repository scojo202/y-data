#include <string.h>
#include <stdio.h>
#include <y-scalar-property.h>

/**
 * SECTION: y-scalar-property
 * @short_description: A scalar data object that reflects the value of a GObject property.
 *
 * Data classes #YScalarProperty
 *
 */

/**
 * YScalarProperty:
 *
 * YScalar that changes when a property changes.
 **/

struct _YScalarProperty {
  YScalar      base;
  GObject *obj;
  gchar *name;
};

G_DEFINE_TYPE (YScalarProperty, y_scalar_property, Y_TYPE_SCALAR);

static void
y_scalar_property_finalize (GObject *obj)
{
  YScalarProperty *s = (YScalarProperty *)obj;
  g_signal_handlers_disconnect_by_data(s->obj,s);
  g_object_unref(s->obj);
  g_free(s->name);

  GObjectClass *obj_class = G_OBJECT_CLASS(y_scalar_property_parent_class);
  (*obj_class->finalize) (obj);
}

static double
y_scalar_property_get_value (YScalar *dat)
{
  YScalarProperty const *sval = (YScalarProperty const *)dat;
  double val;
  g_object_get(sval->obj,sval->name,&val,NULL);
  return val;
}

static void
y_scalar_property_class_init (YScalarPropertyClass *klass)
{
  GObjectClass *gobject_klass = (GObjectClass *) klass;
  gobject_klass->finalize = y_scalar_property_finalize;
  //YDataClass *ydata_klass = (YDataClass *) klass;
  YScalarClass *scalar_klass = (YScalarClass *) klass;
  //ydata_klass->dup  = y_scalar_property_dup;
  scalar_klass->get_value  = y_scalar_property_get_value;
}

static void
y_scalar_property_init(YScalarProperty *val)
{}

static
void on_notify (GObject    *gobject,
                                                        GParamSpec *pspec,
                                                        gpointer    user_data)
{
  YData *d = Y_DATA(user_data);
  y_data_emit_changed(d);
}

/**
 * y_scalar_property_new:
 * @obj: a GObject
 * @name: the property name
 *
 * Creates a new #YScalarProperty object. The property must be numeric.
 *
 * Returns: (transfer full): The new object.
 **/
YScalarProperty	*y_scalar_property_new (GObject *obj, const gchar *name)
{
  /* check that there is a property with this name */
  GObjectClass *class = G_OBJECT_GET_CLASS(obj);
  GParamSpec *spec = g_object_class_find_property(class,name);
  g_return_val_if_fail(spec!=NULL,NULL);
  /* ensure that the property is a numeric type */
  GType type = G_PARAM_SPEC_VALUE_TYPE(spec);
  g_return_val_if_fail(type==G_TYPE_DOUBLE || type==G_TYPE_FLOAT || type==G_TYPE_INT || type==G_TYPE_UINT,NULL);
    
  YScalarProperty *p = g_object_new(Y_TYPE_SCALAR_PROPERTY,NULL);
  p->obj = g_object_ref(obj);
  p->name = g_strdup(name);
  /* connect to notify signal and emit changed */
  gchar *notify = g_malloc(10+strlen(name));
  sprintf(notify,"notify::%s",p->name);
  g_signal_connect(p->obj, notify, G_CALLBACK(on_notify), p);
  g_free(notify);
  return p;
}

