#!/usr/bin/python

from datetime import datetime
from pprint import pprint

from optparse import OptionParser

from sqlalchemy import *
from sqlalchemy.orm import *
from sqlalchemy import func

SQLITE_URL = ""

usage_text = "Usage: %prog [sqlite_database_file]\n" \
		"\n" \
		"If no specified database file - then sqlite :memory: database will be used.\n"

opt_parser = OptionParser(usage=usage_text)

opts, args = opt_parser.parse_args()

if len(args) == 0:
	SQLITE_URL = 'sqlite://'
else:
	url = args[0]
	if url[0] == '.' and url[1] == '/':
		url = url[1:]
	else:
		url = '/' + url

	SQLITE_URL = "sqlite://" + url
	print("Use database file: '" + SQLITE_URL + "'")



################################# CONNECT TO DB

engine = create_engine(SQLITE_URL)

engine.echo = False

meta = MetaData(engine)

################################# CREATE TABLES



if engine.dialect.has_table(engine.connect(), "__schema_version_control"):
	print("Database already have schema.")
	__schema_version_control = Table('__schema_version_control', meta, autoload=True)
else:
	print("Init new database.")
	__schema_version_control = Table('__schema_version_control', meta,
		Column('update_to_version', Integer, primary_key=True),
		Column('timestamp', DateTime, default=datetime.now),
		Column('comment', String, nullable=False)
	)
	__schema_version_control.create()
	i = __schema_version_control.insert()
	i.execute(comment='initial')


s = __schema_version_control.select()
rs = s.execute()
current_schema_revision = max(map(lambda r: r.update_to_version, rs))


def schema_timestamp(comment):
	global current_schema_revision
	i = __schema_version_control.insert()
	i.execute(comment=comment)
	current_schema_revision += 1

print("current_schema_revision:", current_schema_revision)

if current_schema_revision == 1:
	schema_version_comment = "test version"
	print("Update to schema rev. 2 '" + schema_version_comment + "'")

	contact_types = Table('contact_types', meta,
		Column('id', Integer, primary_key=True),

		Column('name', String, nullable=False, unique=True)
	)
	contact_types.create()

	users = Table('users', meta,
		Column('id', Integer, primary_key=True),

		Column('name', String, nullable=False, unique=True),
		Column('comments', String),

		Column('active', Boolean, default=True)
	)
	users.create()

	contacts = Table('contacts', meta,
		Column('id', Integer, primary_key=True),
		Column('user_id', Integer, ForeignKey('users.id')),
		Column('contact_types_id', Integer, ForeignKey('contact_types.id')),

		Column('value', String, nullable=False),
		Column('comments', String),

		Column('active', Boolean, default=True),
		Column('deprecated', Boolean, default=False),
		Column('priority', Integer, default=1),
		Column('public', Boolean, default=False)
	)
	contacts.create()

	nicknames = Table('nicknames', meta,
		Column('id', Integer, primary_key=True),
		Column('user_id', Integer, ForeignKey('users.id')),

		Column('nickname', String, nullable=False),
		Column('comments', String),

		Column('active', Boolean, default=True),
		Column('priority', Integer, default=1)
	)
	nicknames.create()

	schema_timestamp(schema_version_comment)
else:
	contact_types = Table('contact_types', meta, autoload=True)
	users = Table('users', meta, autoload=True)
	contacts = Table('contacts', meta, autoload=True)
	nicknames = Table('nicknames', meta, autoload=True)


if current_schema_revision == 2:
	schema_version_comment = "work version"
	print("Update to schema rev. 3 '" + schema_version_comment + "'")

	currency = Table('currency', meta,
		Column('id', Integer, primary_key=True),

		Column('name', String, nullable=False, unique=True),

		Column('currency_code_num_3', String(3), nullable=False, unique=True),
		Column('currency_code_ascii_3', String(3), nullable=False, unique=True),
		Column('currency_exponent', Integer)
	)
	currency.create()

	period_units = Table('period_units', meta,
		Column('id', Integer, primary_key=True),

		Column('name', String, nullable=False, unique=True)
	)
	period_units.create()
	
	users_payment_obligations = Table('users_payment_obligations', meta,
		Column('id', Integer, primary_key=True),
				   
		Column('user_id', Integer, ForeignKey('users.id'), nullable=False),
		
		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),
		Column('period_unit_id', Integer, ForeignKey('period_units.id')),
		Column('duration', Integer, nullable=False),
		
		Column('changed_timestamp', DateTime, default=datetime.now),
		Column('start_date', DateTime),
		Column('end_date', DateTime)
	)

	regular_fixed_payment_periods = Table('regular_fixed_payment_periods', meta,
		Column('id', Integer, primary_key=True),

		Column('title', String, nullable=False, unique=True),
		Column('comments', String),

		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),
		Column('period_unit_id', Integer, ForeignKey('period_units.id')),
		Column('duration', Integer, nullable=False),
		
		Column('start_date', DateTime),
		Column('end_date', DateTime, nullable=True)
	)
	regular_fixed_payment_periods.create()

	payments = Table('payments', meta,
		Column('id', Integer, primary_key=True),

		Column('title', String, nullable=False, unique=True),
		Column('comments', String),

		Column('regular_fixed_payment_period_id', Integer, ForeignKey('regular_fixed_payment_periods.id'), nullable=True),

		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('approx_amount', Integer, nullable=False)
	)
	payments.create()

	events = Table('events', meta,
		Column('id', Integer, primary_key=True),

		Column('commit_timestamp', DateTime, default=datetime.now),
		Column('timestamp', DateTime),

		# 0 - complex event (sequence of events)
		# 1 - income (user deposit)
		# 2 - payment
		# 3 - slice
		# 4 - merge
		# 5 - exchange
		Column('type', Integer, nullable=False),

		Column('title', String),
		Column('comments', String, nullable=True),

		Column('parent_event_id', Integer, ForeignKey('events.id'), nullable=True),

		Column('src_user_id', Integer, ForeignKey('users.id'), nullable=True),
		Column('dst_payment_id', Integer, ForeignKey('payments.id'), nullable=True),

		Column('exchange_src_currency_id', Integer, ForeignKey('currency.id'),  nullable=True),
		Column('exchange_dst_currency_id', Integer, ForeignKey('currency.id'),  nullable=True),
		Column('exchange_course', Integer, nullable=True),
		Column('exchange_course_exponent', Integer,  nullable=True),
		Column('exchange_location', String, nullable=True)
	)
	events.create()

	money_chunks = Table('money_chunks', meta,
		Column('id', Integer, primary_key=True),

		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),

		Column('alive', Boolean),

		Column('src_event_id', Integer, ForeignKey('events.id'), nullable=True),
		Column('dst_event_id', Integer, ForeignKey('events.id'), nullable=True)
	)
	money_chunks.create()

	__metadata_revisions = Table('__metadata_revisions', meta,
		Column('update_to_version', Integer, primary_key=True),
		Column('timestamp', DateTime, default=datetime.now),
		Column('comment', String, nullable=False)
	)
	__metadata_revisions.create()
	i = __metadata_revisions.insert()
	i.execute(comment='initial')

	schema_timestamp(schema_version_comment)
else:
	currency = Table('currency', meta, autoload=True)
	period_units = Table('period_units', meta, autoload=True)
	regular_fixed_payment_periods = Table('regular_fixed_payment_periods', meta, autoload=True)
	users_payment_obligations = Table('users_payment_obligations', meta, autoload=True)
	payments = Table('payments', meta, autoload=True)
	events = Table('events', meta, autoload=True)
	money_chunks = Table('money_chunks', meta, autoload=True)
	__metadata_revisions = Table('__metadata_revisions', meta, autoload=True)


s = __metadata_revisions.select()
rs = s.execute()
current_metadata_revision = max(map(lambda r: r.update_to_version, rs))

print("current_metadata_revision:", current_metadata_revision)

def metadata_timestamp(comment):
	global current_metadata_revision
	i = __metadata_revisions.insert()
	i.execute(comment=comment)
	current_metadata_revision += 1

################################# INSERT METADATA

if current_metadata_revision == 1:
	metadata_comment = "testing"
	print("Update to metadata rev. 2 '" + metadata_comment + "'")

	i = contact_types.insert()
	i.execute(name='email')
	i.execute({'name': 'skype'},
		{'name': 'jabber'},
		{'name': 'phone'})

	i = currency.insert()
	i.execute(name='Белорусский рубль', currency_code_num_3='974', currency_code_ascii_3='BYR', currency_exponent=0)
	i.execute(name='Доллар США', currency_code_num_3='840', currency_code_ascii_3='USD', currency_exponent=2)
	i.execute(name='Российский рубль', currency_code_num_3='643', currency_code_ascii_3='RUB', currency_exponent=2)
	i.execute(name='Евро', currency_code_num_3='978', currency_code_ascii_3='EUR', currency_exponent=2)

	i = period_units.insert()
	i.execute(name='день')
	i.execute(name='неделя')
	i.execute(name='месяц')
	i.execute(name='год')

	metadata_timestamp(metadata_comment)


################################# OBJECTS MAPPING, SESSIONS AND RELATIONS

print("\n")


class User(object):
	def __init__(self, name=None, contacts=[]):
		self.name = name
		self.contacts = contacts
	def __repr__(self):
		return self.name

class ContactType(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name

class Contact(object):
	def __init__(self, type=None, value=None):
		self.value = value
		self.type = session.query(ContactType).filter_by(name=type).first()
	def __repr__(self):
		return self.value

contact_mapper = mapper(Contact, contacts,
			properties={
				'user' : relation(User),
				'type' : relation(ContactType)
			}
		)

contact_types_mapper = mapper(ContactType, contact_types,
			properties={'contacts': relation(contact_mapper)}
		)

user_mapper = mapper(User, users,
			properties={'contacts': relation(contact_mapper)}
		)

class Currency(object):
	def __init__(self, name=None, currency_code_num_3=None, currency_code_ascii_3=None, currency_exponent=0):
		self.name = name
		self.currency_code_num_3 = currency_code_num_3
		self.currency_code_ascii_3 = currency_code_ascii_3
		self.currency_exponent = currency_exponent
	def __repr__(self):
		return self.currency_code_ascii_3
	def f_val(value):
		return value / (10**self.currency_exponent)
	def s_val(value):
		return ("%."+("%d" % self.currency_exponent)+"f") % self.f_val(value) + " " + self.currency_code_ascii_3
 
class PeriodUnit(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name
	
class UserPaymentObligation(object):
	def __init__(self, user, currency=None, amount=0, period_unit=None, duration=0, start_date=None, end_date=None):
		self.user = user
		self.currency = currency if currency else session.query(Currency).filter_by(currency_code_ascii_3='BYR').first()
		self.amount = amount
		self.period_unit = period_unit if period_unit else session.query(PeriodUnit).filter_by(name='месяц').first()
		self.duration = duration
		self.start_date = start_date if start_from else datetime.now
		self.end_date = end_date
	def __repr__(self):
		return self.currency.s_val(self.amount) + " / " + self.duration + " " + self.period_unit + " from " + self.start_date + 
			((" till " + self.end_date) if self.end_date else "")

class RegularFixedPaymentPeriod(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name

class Payment(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name

class Event(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name

class MoneyChunk(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name



#TODO: отдельные файлы: подключение к базе и настройки, создание и модификация схемы, metadata, тесты схемы, классы, тесты классов, функции-запросы, тесты функций, сервер, тесты сервера

################################# TESTING

session = create_session()

session.merge(User(name='Oleg'))

session.flush()





new_user = User(name='Yri', contacts=[Contact(type='phone', value="123456789")])

session.merge(new_user)


session.flush()



all_users = session.query(User)
for user in all_users:
	print(user.name)
	for contact in sorted(user.contacts, key=lambda c: c.priority):
		print('    ', contact.type, ":", contact)


all_contacts = session.query(Contact)
for c in all_contacts:
	session.delete(c)

all_users = session.query(User)
for u in all_users:
	session.delete(u)







