#!/usr/bin/python

from datetime import datetime
from pprint import pprint

from optparse import OptionParser

from sqlalchemy import *
from sqlalchemy.orm import *
from sqlalchemy import func

SQLITE_URL = ""

################################# COMMAND-LINE PARAMETERS

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

		Column('code_num_3', String(3), nullable=False, unique=True),
		Column('code_ascii_3', String(3), nullable=False, unique=True),
		Column('exponent', Integer)
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
		Column('comments', String),
		
		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),
		Column('period_unit_id', Integer, ForeignKey('period_units.id')),
		Column('duration', Integer, nullable=False),

		Column('start_date', DateTime),
		Column('end_date', DateTime)
	)

	payments = Table('payments', meta,
		Column('id', Integer, primary_key=True),

		Column('title', String, nullable=False, unique=True),
		Column('comments', String)
	)
	payments.create()

	payment_periods = Table('payment_periods', meta,
		Column('id', Integer, primary_key=True),

		Column('payment_id', Integer, ForeignKey('payments.id'), nullable=False),

		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),
		Column('period_unit_id', Integer, ForeignKey('period_units.id')),
		Column('duration', Integer, nullable=True),
		
		Column('start_date', DateTime),
		Column('end_date', DateTime, nullable=True)
	)
	payment_periods.create()

	exchange_event_data = Table('exchange_event_data', meta,
		Column('id', Integer, primary_key=True),

		Column('src_currency_id', Integer, ForeignKey('currency.id'),  nullable=True),
		Column('dst_currency_id', Integer, ForeignKey('currency.id'),  nullable=True),
		Column('course', Integer, nullable=True),
		Column('course_exponent', Integer,  nullable=True),
		Column('location', String, nullable=True)
	)

	events = Table('events', meta,
		Column('id', Integer, primary_key=True),

		Column('commit_timestamp', DateTime, default=datetime.now),
		Column('event_timestamp', DateTime),

		# 0 - complex event (sequence of events) - several events has it's id as parent_event_id
		# 1 - income (user deposit) - src_user_id not Null
		# 2 - payment - dst_payment_id not Null
		# 3 - slice - utility event, parent_event_id not Null
		# 4 - merge - utility event, parent_event_id not Null
		# 5 - exchange - exchange_event_data_id not Null
		Column('type', Integer, nullable=False),

		Column('comments', String, nullable=True),

		Column('parent_event_id', Integer, ForeignKey('events.id'), nullable=True),

		Column('src_user_id', Integer, ForeignKey('users.id'), nullable=True),
		Column('dst_payment_id', Integer, ForeignKey('payments.id'), nullable=True),

		Column('exchange_event_data_id', Integer, ForeignKey('exchange_event_data.id'), nullable=True)
	)
	events.create()

	money_chunks = Table('money_chunks', meta,
		Column('id', Integer, primary_key=True),

		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),

		Column('src_event_id', Integer, ForeignKey('events.id'), nullable=False),
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
	users_payment_obligations = Table('users_payment_obligations', meta, autoload=True)
	payment_periods = Table('regular_fixed_payment_periods', meta, autoload=True)
	payments = Table('payments', meta, autoload=True)
	exchange_event_data = Table('exchange_event_data', meta, autoload=True)
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
	i.execute(name='Белорусский рубль', code_num_3='974', code_ascii_3='BYR', exponent=0)
	i.execute(name='Доллар США', code_num_3='840', code_ascii_3='USD', exponent=2)
	i.execute(name='Российский рубль', code_num_3='643', code_ascii_3='RUB', exponent=2)
	i.execute(name='Евро', code_num_3='978', code_ascii_3='EUR', exponent=2)

	i = period_units.insert()
	i.execute(name='день')
	i.execute(name='неделя')
	i.execute(name='месяц')
	i.execute(name='год')

	metadata_timestamp(metadata_comment)


################################# OBJECTS MAPPING AND RELATIONS

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

def num_w_exp (value, expanant):
	value / (10**exponent)

def num_str_exp (value, expanent):
	("%."+("%d" % exponent)+"f") % num_w_exp(value, expanent)

class Currency(object):
	def __init__(self, name=None, code_num_3=None, code_ascii_3=None, exponent=0):
		self.name = name
		self.code_num_3 = code_num_3
		self.code_ascii_3 = code_ascii_3
		self.exponent = exponent
	def __repr__(self):
		return self.code_ascii_3
	def f_val(value):
		return num_w_exp(value, self.expanent)
	def s_val(value):
		return num_str_exp(value, self.expanent) + " " + self.code_ascii_3
 
class PeriodUnit(object):
	def __init__(self, name=None):
		self.name = name
	def __repr__(self):
		return self.name

class UserPaymentObligation(object):
	def __init__(self, user, currency=None, amount=0, period_unit=None, duration=0, start_date=None, end_date=None):
		self.user = user
		self.currency = currency if currency else session.query(Currency).filter_by(code_ascii_3='BYR').first()
		self.amount = amount
		self.period_unit = period_unit if period_unit else session.query(PeriodUnit).filter_by(name='месяц').first()
		self.duration = duration
		self.start_date = start_date if start_from else datetime.now
		self.end_date = end_date
	def __repr__(self):
		return self.currency.s_val(self.amount) + " / " + self.duration + " " + self.period_unit + " from " + self.start_date + ((" till " + self.end_date) if self.end_date else "")

class Payment(object):
	def __init__(self, title=None):
		self.title = title
	def __repr__(self):
		return self.title

class PaymentPeriod(object):
	def __init__(self, payment, currency=None, amount=0, period_unit=None, duration=0, start_date=None, end_date=None):
		self.payment = payment
		self.currency = currency if currency else session.query(Currency).filter_by(code_ascii_3='BYR').first()
		self.amount = amount
		self.period_unit = period_unit if period_unit else session.query(PeriodUnit).filter_by(name='месяц').first()
		self.duration = duration
		self.start_date = start_date if start_from else datetime.now
		self.end_date = end_date
	def __repr__(self):
		if self.duration != 0:
			return self.currency.s_val(self.amount) + " / " + self.duration + " " + self.period_unit + " from " + self.start_date + ((" till " + self.end_date) if self.end_date else "")
		else:
			return self.currency.s_val(self.amount) + " till " + self.end_date

class ExchangeEventData(object):
	def __init__(self, src_currency, dst_currency, course, exponent, location=None):
		self.src_currency = src_currency
		self.dst_currency = dst_currency
		self.course = course
		self.exponent = exponent
		self.location = location
	def __repr__(self):
		return self.src_currency + "/" + self.dst_currency + " " + num_str_exp(self.course, self.expanent)

class Event(object):
	def type_str (type_num):
		return {
			0 : "complex",
			1 : "income",
			2 : "payment",
			3 : "slice",
			4 : "merge",
			5 : "exchange"
			}[type_num];

	def type_id (type_str):
		return {
			"complex" : 0,
			"income" : 1,
			"payment" : 2,
			"slice" : 3,
			"merge" : 4,
			"exchange" : 5
			}[type_str];

	def type_name (self):
		return type_str(self.type)

	def __init__(self, type, event_timestamp=None, parent_event=None, src_user=None, dst_payment=None, exchange_event_data=None):
		self.type = type
		self.event_timestamp = event_timestamp
		self.parent_event = parent_event
		self.src_user = src_user
		self.dst_payment = dst_payment
		self.exchange_event_data = exchange_event_data
	def __repr__(self):
		return self.type_name() + " " + self.event_timestamp + {
			0 : "",
			1 : " " + self.src_user,
			2 : " " + self.dst_payment,
			3 : "",
			4 : "",
			5 : " " + self.exchange_event_data
			} [self.type]

class MoneyChunk(object):
	def __init__(self, currency, amount, src_event):
		self.currency = currency
		self.amount = amount
		self.src_event = src_event
		self.dst_event = None
	def __repr__(self):
		return self.name


period_unit_mapper = mapper(PeriodUnit, period_units)

user_payment_obligation_mapper = mapper(UserPaymentObligation, users_payment_obligations,
			properties={
				'user' : relation(User),
				'currency' : relation(Currency),
				'period_unit' : relation(PeriodUnit)
			}
		)

payment_period_mapper = mapper(PaymentPeriod, payment_periods,
			properties={
				'payment' : relation(Payment),
				'currency' : relation(Currency),
				'period_unit' : relation(PeriodUnit)
			}
		)

exchange_event_data_mapper = mapper(ExchangeEventData, exchange_event_data,
			properties={
				'src_currency' : relation(Currency),
				'dst_currency' : relation(Currency)
			}
		)

money_chunk_mapper = mapper(MoneyChunk, money_chunks,
			properties={
				'currency' : relation(Currency),
				'src_event' : relation(Event),
				'dst_event' : relation(Event),
			}
		)

event_mapper = mapper(Event, events,
			properties={
				'parent_event' : relation(Event),
				'src_user' : relation(User),
				'dst_payment' : relation(Payment),
				'exchange_event_data' : relation(ExchangeEventData),

				'child_events' : relation(event_mapper),
				'money_chunks_src' : relation(money_chunk_mapper.src_event),
				'money_chunks_dst' : relation(money_chunk_mapper.dst_event)
			}
		)


payment_mapper = mapper(Payment, payments,
			properties={
				'periods' : relation(payment_period_mapper),
				'events' : relation(event_mapper)
			}
		)

currency_mapper = mapper(Currency, currency,
			properties={
				'money_chunks' : relation(money_chunk_mapper),
				'user_payment_obligations' : relation(user_payment_obligation_mapper),
				'exchange_event_data_src' : relation(exchange_event_data_mapper.src_currency),
				'exchange_event_data_dst' : relation(exchange_event_data_mapper.dst_currency),
				'payment_periods' : relation(payment_period_mapper)
			}
		)



#TODO: операции для чисел с экспонентой

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







