import turbogears
from nc import model
from sqlobject import *
hub = __connection__ = model.hub
from datetime import datetime
from turbogears import config
from registration.ormmanager import count

def create_registration_tables():
    "Create the appropriate database tables."
    RegistrationPendingUser.createTable(ifNotExists=True)
    RegistrationUserEmailChange.createTable(ifNotExists=True)
    RegistrationUserLostPassword.createTable(ifNotExists=True)

# Automatically create the registration tables when TurboGears starts up
# turbogears.startup.call_on_startup.append(create_registration_tables)


_user_path = config.get('identity.soprovider.model.user',
                                        'nc.model.User')
User = turbogears.util.load_class(_user_path)
                                        

class RegistrationPendingUser(SQLObject):
    """Model to store information about users that have not been verified.
    """
    # This class should be modified to essentially look like your main model.User
    # class.  You don't need to add in the group information
    user_name = UnicodeCol( length=16, alternateID=True,
                           alternateMethodName="by_user_name" )
    email_address = UnicodeCol( length=255, alternateID=True,
                               alternateMethodName="by_email_address" )
    display_name = UnicodeCol( length=255 )
    password = UnicodeCol( length=40 )
    created = DateTimeCol( default=datetime.now )
    validation_key = StringCol( length=40 )

class RegistrationUserEmailChange(SQLObject):
    """Model to store changes to user email addresses for validation.
    """
    user = ForeignKey(User.__name__)
    new_email_address = UnicodeCol(length=255, alternateID=True,
                                    alternateMethodName='by_new_email_address')
    validation_key = StringCol(length=40)
    created = DateTimeCol(default=datetime.now)
    
class RegistrationUserLostPassword(SQLObject):
    """Model to store info about lost password status."""
    
    user = ForeignKey(User.__name__, unique=True)
    validation_key = StringCol(length=40)
    created = DateTimeCol(default=datetime.now)
    
    @classmethod
    def get_by_username_key(cls, user_name, key):
        try:
            user = User.by_user_name(user_name)
        except SQLObjectNotFound:
            return None
        
        select = cls.select(AND(cls.q.userID==user.id,
                                cls.q.validation_key==key))    
        return select.getOne(default=None)
    


def user_name_is_unique(user_name):
    "Return True if the user_name is not yet in the database."
    user_count = count(User, user_name=user_name)
    pending_count = count(RegistrationPendingUser, user_name=user_name)
    return not(user_count or pending_count)

        
def email_is_unique(email):
    "Return True if the email is not yet in the database."
    user_count = count(User, email_address=email)
    pending_count = count(RegistrationPendingUser, email_address=email)
    changed_count = count(RegistrationUserEmailChange,
                            new_email_address=email)
    return not(user_count or pending_count or changed_count)
