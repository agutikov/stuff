#!/usr/bin/python





'''

	юзвери
	у каждого юзверя есть интервалы времени когда он "учавствует" и когда нет - т.е. когда он должен сдавать и когда нет
		это обобщение варианта когда записывается только дата вступления и выбытия из рядов
	на каждом интервале юзверь может иметь разный размер "обещанного взноса" - т.е. суммы которую он подписался платить ежемесячно
	никто ему не мешает взносить больше - тогда то что больше - можно раскидать на определённые месяцы или оставить за тот в который произведён взнос

	таймлайн - квантование по дням
	юзвери сдают деньги в какой-то день в какой-то валюте
	они зачисляются на "личный счёт" но фактически являются уже общаком - и включаются в него при подсчётах

	таким образом общак считается во всех валютах сразу

	далее - за что-то надо платить
	и тут появляются затратные статьи - такие как аренда, ремонт, и т.д.
	среди них есть обязательные и необязательные
	т.е. кто-то может подписаться или нет на необязательные
	но! - оплата, в отличии от взносов, производится всегда в одной определённой валюте

	далее в какой-то день может произойти обмен валют - тогда тот кто менял общаковые бабки вносит в историю запись о том где когда и сколько менял
	если не вся определённая валюта обменяна - то считается что с каждого лчиного счёта снята пропорциональная сумма
	и вот тогда надо чтобы можно было проследить с кого сколько взяли в какой валюте и на что поменяли

	а потом поменянные деньги и непоменянные - оправляются на оплату чего-либо
	и фактически фиксируются в виде оплаты

	т.е. всегда можно проследить - сколько кто внёс, сколько из этого на что и когда поменяли и сколько на что и когда потратили

	----

	!!! важный технический момент

	данные обрабатываются в sqlite с клиентом на питоне
	хранятся в виде дампа в sql с контролем версий через git
	интерфейс как у fdisk - консольный интерактивный (не коммандный для начала)

	потом можно будет сделать сервак и клиент с коммандным интерфейсом
	и все остальные свистелки типа curses, gui, webgui и т.д.






	----

	вариант со счетами и обезличенной валютой не прокатит - надо рисовать куски денег с собственной историей:
	от кого когда поступили, когда дробились или объединялись, когда и на что менялись, когда и на что потратились
	дробить и соединять можно куски денег только одной валюты
	менять можно только разные валюты одну на другую - всегда определённые куски

	первое что приходит в голову - тут обязательно понадобятся рациональные числа

	юзверь:
		интервалы
		взносы
	интервал: начало, конец, обещанная сумма
	сумма: валюта, рациональная сумма
	кусок денег: валюта, рациональная сумма, id
	операции:
		взнос - дата, юзер, кусок денег,
		деление - начальный кусок денег - получившиеся куски,
		слияние - начальные куски - получившийся новый кусок денег,
		обмен - начальные кусок в одной валюте - итоговый в другой, плюс дата, место, курс и т.д.,
		расход - кусок денег в одной валюте, дата, описание
		составная операция - имеющая логический смысл оперция состоящая из нескольких атомарных например деление и слияний



'''




'''
	параметры коммандной строки
	конфиг

	дальше находим репозитарий и вытаскиваем оттуда files.list
	создаем базу sqlite в памяти и грузим туда файлы из списка по очереди
	сначала файлик создающий таблицы, потом данные для каждой таблицы

	база готова - выводим инфу и пошли в бесконечный цикл интерфейса

	каждая команда реализована в отдельной функции
	парсинг параметров общий
	есть список функций с описанием, и метаданными о парамтерах
	и мап имён комманд на описания функций
	функции могут быть активными и неактивными в зависимости от того загружена база или нет
	при первом запуске когда базы ещё нет выводится пояснение как её создать - специальной командой

	после выполнения каждой изменяющей функции делается дамп базы вручную:
	данные по каждой таблице в отдельном файле, создание таблиц отдельно, и список файлов
	далее это кладется в гит и коммитится

	изменяющие функции:
		взнос
		обмен
		расход

	неизменяющие:
		кто сколько когда сдал в какой валюте
		кто сколько когда сдал в переводе на конкретную валюту (для отчётов например)
		и т.д.

'''


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



if engine.dialect.has_table(engine.connect(), "schema_version_control"):
	print("Database already have schema.")
	schema_version_control = Table('schema_version_control', meta, autoload=True)
else:
	print("Init new database.")
	schema_version_control = Table('schema_version_control', meta,
		Column('update_to_version', Integer, primary_key=True),
		Column('timestamp', DateTime, default=datetime.now),
		Column('comment', String, nullable=False)
	)
	schema_version_control.create()
	i = schema_version_control.insert()
	i.execute(comment='initial')


s = schema_version_control.select()
rs = s.execute()
current_schema_revision = max(map(lambda r: r.update_to_version, rs))


def schema_timestamp(comment):
	global current_schema_revision
	i = schema_version_control.insert()
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

	regular_fixed_payment_periods = Table('regular_fixed_payment_periods', meta,
		Column('id', Integer, primary_key=True),

		Column('title', String, nullable=False, unique=True),
		Column('comments', String),

		Column('currency_id', Integer, ForeignKey('currency.id')),
		Column('amount', Integer, nullable=False),

		Column('period_unit_id', Integer, ForeignKey('period_units.id')),
		Column('duration', Integer, nullable=False)
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
		Column('comments', String),

		Column('parent_event_id', Integer, ForeignKey('events.id'), nullable=True),

		Column('src_user_id', Integer, ForeignKey('users.id'), nullable=False),
		Column('dst_payment_id', Integer, ForeignKey('payments.id'), nullable=False),

		Column('exchange_src_currency_id', Integer, ForeignKey('currency.id')),
		Column('exchange_dst_currency_id', Integer, ForeignKey('currency.id')),
		Column('exchange_course', Integer, nullable=False),
		Column('exchange_course_exponent', Integer, nullable=False),
		Column('exchange_location', String)
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

	metadata_revisions = Table('metadata_revisions', meta,
		Column('update_to_version', Integer, primary_key=True),
		Column('timestamp', DateTime, default=datetime.now),
		Column('comment', String, nullable=False)
	)
	metadata_revisions.create()
	i = metadata_revisions.insert()
	i.execute(comment='initial')

	schema_timestamp(schema_version_comment)
else:
	currency = Table('currency', meta, autoload=True)
	period_units = Table('period_units', meta, autoload=True)
	regular_fixed_payment_periods = Table('regular_fixed_payment_periods', meta, autoload=True)
	payments = Table('payments', meta, autoload=True)
	events = Table('events', meta, autoload=True)
	money_chunks = Table('money_chunks', meta, autoload=True)
	metadata_revisions = Table('metadata_revisions', meta, autoload=True)


s = metadata_revisions.select()
rs = s.execute()
current_metadata_revision = max(map(lambda r: r.update_to_version, rs))

print("current_metadata_revision:", current_metadata_revision)

def metadata_timestamp(comment):
	global current_metadata_revision
	i = metadata_revisions.insert()
	i.execute(comment=comment)
	current_metadata_revision += 1

################################# INSERT METADATA


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

################################# SELECT

s = contact_types.select()
rs = s.execute()

for row in rs:
	print(row.id, row.name)

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













