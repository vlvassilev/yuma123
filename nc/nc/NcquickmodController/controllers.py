#coding=utf-8
from turbogears import controllers, expose, redirect
from turbogears import validate, flash, error_handler
from nc.model import Ncquickmod
from turbogears import widgets, validators
from turbogears import identity
from turbogears import paginate

class NcquickmodFields(widgets.WidgetsList):
    """fields definitions. Replace to your Fields"""
    modname = widgets.TextField(label="Module Name")

class NcquickmodSchema(validators.Schema):
    """
    define validation schema that involves 
    field dependency or logical operators
    """
    modname = validators.String(not_empty=True, max=64)

class NcquickmodForm(widgets.TableForm):
    """form builder"""
    name="Ncquickmod"
    fields = NcquickmodFields()
    validator = NcquickmodSchema() # define schema outside of NcquickmodFields
    method="post"
    submit_text = "Create"

model_form = NcquickmodForm()

#protect NcquickmodController with identity by include
class NcquickmodController(controllers.Controller, identity.SecureResource):
    require = identity.in_group("admin")

    """Basic model admin interface"""
    modelname="Ncquickmod"

    @expose()
    def default(self, tg_errors=None):
        """handle non exist urls"""
        raise redirect("list")

    require = identity.in_group("admin")
    @expose()

    def index(self): 
    	"""handle front page"""
    	raise redirect("list")

    @expose(template='kid:nc.NcquickmodController.templates.list')
    @paginate('records')
    def list(self, **kw):
        """List records in model"""
        records = Ncquickmod.select()
        return dict(records = records, modelname=self.modelname)

    @expose(template='kid:nc.NcquickmodController.templates.show')
    def show(self,id, **kw):
        """Show record in model"""
        record = Ncquickmod.get(int(id))
        return dict(record = record)

    @expose(template='kid:nc.NcquickmodController.templates.form')
    def new(self, **kw):
        """Create new records in model"""
        return dict(modelname = self.modelname, form = model_form, page='new')

    @expose(template='kid:nc.NcquickmodController.templates.form')
    def edit(self, id, **kw):
        """Edit record in model"""
        try:
            record = Ncquickmod.get(int(id))
        except:
            flash = "Not valid edit"
        return dict(modelname = self.modelname, form = model_form, page='edit',
                    record = record)

    @validate(model_form)
    @error_handler(new)
    @expose()
    def save(self, id=None, **kw):
        """Save or create record to model"""
        #update kw
        if id:
            #do update
            record = Ncquickmod.get(int(id))
            for attr in kw:
                setattr(record, attr, kw[attr])
            flash("Ncquickmod was successfully updated.")
            raise redirect("../list")
        else:
            #do create
            Ncquickmod(**kw)
            flash("Ncquickmod was successfully created.")
            raise redirect("list")

    @expose()
    def destroy(self, id):
        """Destroy record in model"""
        record = Ncquickmod.get(int(id))
        record.destroySelf()
        flash("Ncquickmod was successfully destroyed.")
        raise redirect("../list")
