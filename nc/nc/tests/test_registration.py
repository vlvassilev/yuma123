from turbogears import config, database, controllers, testutil, startup
from nc import model
from nc.registration import model as register_model
from nc.registration import controllers as register_controllers
import cherrypy
import urllib
import unittest
from os import environ
from registration.ormmanager import create, retrieve_one, count


# please update these lines appropriately
mail_server = environ.get('REGTST_EMAIL_SERVER', 'localhost')
test_email = environ.get('REGTST_TEST_EMAIL', 'plewis@example.com')
new_email = environ.get('REGTST_NEW_EMAIL', 'pat.lewis@example.com')
admin_email = environ.get('REGTST_ADMIN_EMAIL', 'admin@example.com')

config.update({'global':{'registration.mail.smtp_server': mail_server,
                        'registration.mail.admin_email': admin_email}})

config.update({'global': {
                    'visit.on':True,
                    'visit.identity': 'sqlobject',
                    'identity.on':True }
                })

RegistrationPendingUser = register_model.RegistrationPendingUser
RegistrationUserEmailChange = register_model.RegistrationUserEmailChange
RegistrationUserLostPassword = register_model.RegistrationUserLostPassword


# This probably looks a bit odd, but the SharedMethods class is 'shared'
# by both SQLAlchemy tests and SQLObject tests. You are only seeing one
# side of that (either SA or SO).

class SharedMethods(object):
    "Container for methods that can be shared in SQLAlchemy and SQLObject test cases."
    
    def create_pending_user_by_request(self):
        req_str = ''.join(['/register/create?user_name=%s&email=%s&email2=%s',
                            '&display_name=%s&password1=%s&password2=%s']) % \
                            ('dschrute', test_email, test_email, 'Dwight+Schrute',
                            'secret', 'secret')
        testutil.create_request(req_str)
        
    def assert_ok_response(self):
        if cherrypy.response.status != '200 OK':
            raise AssertionError, cherrypy.response.body[0]
            
    def create_pending_user(self):
        pend = create(RegistrationPendingUser,
                        user_name='dschrute',
                        email_address=test_email,
                        display_name='Dwight Schrute',
                        password='secret',
                        validation_key='0123456789')
        return pend
    
    def create_user(self):
        usr = create(register_model.User, user_name='dschrute',
                                        email_address=test_email,
                                        display_name='Dwight Schrute',
                                        password='secret')
        return usr
        
    def test_new_registration(self):
        "A new pending user is created."
        self.create_pending_user_by_request()
        pend = retrieve_one(RegistrationPendingUser, user_name='dschrute')
        self.assert_ok_response()
        self.assertEqual(pend.email_address, test_email)
        assert(count(register_model.User)==0)
        
    def test_user_validation(self):
        "A pending user is promoted to an actual user on validation."
        self.create_pending_user()
        assert(count(RegistrationPendingUser)==1)
        req_str = '/register/validate_new_user?email=%s&key=%s' % (test_email, '0123456789')
        testutil.create_request(req_str)
        self.assert_ok_response()
        assert(count(RegistrationPendingUser)==0)
        usr = retrieve_one(register_model.User, email_address=test_email)
        assert(usr.user_name=='dschrute')
        
    def test_in_groups_on_validation(self):
        "A new (validated) user is placed in the appropriate groups."
        # create the Group 
        create(model.Group,
            group_name='dojo_members', display_name='Members of the dojo')
        config.update({'global':{'registration.verified_user.groups':['dojo_members']}})
        self.create_pending_user()
        req_str = '/register/validate_new_user?email=%s&key=%s' % (test_email, '0123456789')
        testutil.create_request(req_str)
        usr = retrieve_one(register_model.User, email_address=test_email)
        assert(len(usr.groups)==1)
        assert(usr.groups[0].group_name=='dojo_members')
        
    def test_change_password(self):
        "A logged in user can change their password."
        usr = self.create_user()
        params = dict(email=test_email,
                        old_password='secret',
                        password1='supersecret',
                        password2='supersecret',
                        # next three fields are to authenticate the user with identity
                        user_name='dschrute',
                        password='secret',
                        login='login')
        req_str = '/register/update_user?%s' % urllib.urlencode(params)
        testutil.create_request(req_str)
        usr = retrieve_one(register_model.User, email_address=test_email)
        assert(usr.password=='supersecret')
                
    def test_changed_email(self):
        "A logged in user can change their email address."
        usr = self.create_user()
        params = dict(email=new_email,
                        old_password='secret',
                        password1='',
                        password2='',
                        user_name=usr.user_name,
                        password=usr.password,
                        login='login')
        req_str = '/register/update_user?%s' % urllib.urlencode(params)
        testutil.create_request(req_str)
        # email should still be unchanged at this point, but there should be an
        # entry in the RegistrationUserEmailChange table
        usr = retrieve_one(register_model.User, user_name=usr.user_name)
        assert(usr.email_address==test_email)
        email_change = retrieve_one(RegistrationUserEmailChange, new_email_address=new_email)
        assert(email_change.user.user_name==usr.user_name)
        # Ok, now the user validates his new email address
        req_str = '/register/validate_email_change?%s' % \
                    urllib.urlencode({'email':new_email, 'key':email_change.validation_key})
        email_change = None
        testutil.create_request(req_str)
        usr = retrieve_one(register_model.User, user_name=usr.user_name)
        assert(count(RegistrationUserEmailChange)==0)
        assert(usr.email_address==new_email)    
    
    def test_lost_password_page(self):
        "The lost password page returns."
        url = '/register/lost_password'
        testutil.create_request(url)
        self.assert_ok_response()
        assert(cherrypy.response.body[0].find('<h1>Lost Password</h1>') > 0)
        
    def test_reset_password_request(self):
        "A user can request that their password be reset."
        usr = self.create_user()
        req_str = '/register/mail_lost_password_url?email_or_username=%s' % test_email
        testutil.create_request(req_str)
        self.assert_ok_response()
        
    def test_reset_password_works(self):
        "With the proper url, a user can reset their password."
        usr = self.create_user()
        key = 'abcdefghijklmnop'
        pwd = 'newpassword'
        create(RegistrationUserLostPassword, user=usr, validation_key=key)
        req_str = ''.join(('/register/reset_password_save?',
                    'user_name=%s&key=%s&password1=%s&',
                    'password2=%s&display_user_name=%s')) % (
                        usr.user_name, key, pwd, pwd, usr.user_name)
        testutil.create_request(req_str.encode('ascii'))
        # reset_password_save raises a redirect, so dont test the response
        #self.assert_ok_response()
        assert cherrypy.response.status == '302 Found', cherrypy.response.status
        usr = retrieve_one(register_model.User, user_name=usr.user_name)
        assert usr.password == pwd, usr.password
        
        
    def test_reset_password_fails(self):
        "Without proper credentials, a user cannot reset their password."
        usr = self.create_user()
        key = '1234567890'
        pwd = 'something_new'
        create(RegistrationUserLostPassword, user=usr, validation_key=key)
        req_str = ''.join(('/register/reset_password_save?',
                    'user_name=%s&key=%s__BAD__&password1=%s&',
                    'password2=%s&display_user_name=%s')) % (
                        usr.user_name, key, pwd, pwd, usr.user_name)
        testutil.create_request(req_str.encode('ascii'))
        self.assert_ok_response()
        original_pwd = usr.password
        usr = retrieve_one(register_model.User, user_name=usr.user_name)
        assert usr.password == original_pwd
        

#database.set_db_uri('sqlite:///:memory:')

config.update({'global': {
        'visit.soprovider.model':'turbogears.visit.sovisit.TG_Visit',
        'identity.soprovider.model.user':"nc.model.User",
        'identity.soprovider.model.group':"nc.model.Group",
        'identity.soprovider.model.permission':"nc.model.Permission",
        'sqlobject.dburi': 'sqlite:///:memory:'
        }})



class TestSORegistrationModel(testutil.DBTest, SharedMethods):
    model = register_controllers.register_model  # hopefully, register_model still imports model
    
    def setUp(self):
        self.create_identity_tables()
        register_model.create_registration_tables()
        cherrypy.root = None
        cherrypy.tree.mount_points = {}
        cherrypy.tree.mount(RegTestController(), '/')
        config.update({'global':{'registration.verified_user.groups':[],
                                'registration.unverified_user.groups':[] }
                        })
        
    def tearDown(self):
        self.drop_identity_tables()
        RegistrationPendingUser.dropTable(ifExists=True)
        RegistrationUserEmailChange.dropTable(ifExists=True)
        RegistrationUserLostPassword.dropTable(ifExists=True)
        startup.stopTurboGears()
                
    def test_pending_user_groups(self):
        "A unvalidated user is made a real user in the unvalidated group(s)."
        # create the group
        model.Group(group_name='dojo_visitors', display_name='Visitors to the dojo')
        model.Group(group_name='dojo_members', display_name='Members of the dojo')
        config.update({'global':{'registration.unverified_user.groups':['dojo_visitors'],
                                    'registration.verified_user.groups':['dojo_members']}})
        self.create_pending_user_by_request()
        self.assert_ok_response()
        pend = RegistrationPendingUser.by_email_address(test_email)
        # A new user should now be created, and should have the unverified groups
        usr = retrieve_one(register_model.User, email_address=test_email)
        assert(len(usr.groups)==1)
        assert(usr.groups[0].group_name=='dojo_visitors')
        # ok, now lets validate this user.
        req_str = '/register/validate_new_user?email=%s&key=%s' % (test_email, pend.validation_key)
        pend = None
        testutil.create_request(req_str)
        usr.sync()
        assert(RegistrationPendingUser.select().count()==0)
        assert(len(usr.groups)==1)
        assert(usr.groups[0].group_name=='dojo_members')
                                
    def create_identity_tables(self):
        register_model.User.createTable(ifNotExists=True, createJoinTables=False)
        model.Permission.createTable(ifNotExists=True, createJoinTables=False)
        model.Group.createTable(ifNotExists=True, createJoinTables=True)
        model.VisitIdentity.createTable(ifNotExists=True)
        
    def drop_identity_tables(self):
        model.VisitIdentity.dropTable(ifExists=True)
        model.Group.dropTable(ifExists=True, dropJoinTables=True)
        model.Permission.dropTable(ifExists=True, dropJoinTables=True)
        register_model.User.dropTable(ifExists=True)


class RegTestController(controllers.RootController):
    
    register = register_controllers.UserRegistration()