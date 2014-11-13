from django.db import models
from datetime import datetime, date, timedelta
from dateutil.relativedelta import relativedelta
import calendar

def num_w_exp (value, exponent):
	return value / (10**exponent)

def num_str_exp (value, exponent):
	return ("%."+("%d" % exponent)+"f") % num_w_exp(value, exponent)

DAY = 'd'
WEEK = 'w'
MONTH = 'm'
YEAR = 'y'

PERIOD_UNIT_CHOICES = (
	(DAY, 'day'),
	(WEEK, 'week'),
	(MONTH, 'month'),
	(YEAR, 'year'),
)


BEFORE = 'b'
AT = 'a'
AFTER = 'f'

UNTIL_CONDITION = (
	(BEFORE, 'before'),  #   |  pay  |   use   |   -   |
	(AT, 'at'),          #   |   -   | pay&use |   -   |
	(AFTER, 'after'),    #   |   -   |   use   |  pay  |
)


COMPLEX = 'c' # complex event (sequence of events) - several events has it's id as parent
INCOME = 'i' # income (subject deposit) - src not Null
PAYMENT = 'p' # payment - dst not Null
SLICE = 's' # slice - utility event, delete one and create 2 or more chunks
MERGE = 'm' # merge - utility event, delete 2 or more and create 1 chunk
EXCHANGE = 'e' # exchange - exchange_event_data not Null

EVENT_TYPE_CHOICES = (
	(COMPLEX, 'complex'),
	(INCOME, 'income'),
	(PAYMENT, 'payment'),
	(SLICE, 'slice'),
	(MERGE, 'merge'),
	(EXCHANGE, 'exchange'),
)

# end - start in month
def diff_month(end, start):
	sign = 1
	if end < start:
		sign = -1
		tmp = start
		start = end
		end = tmp
	difference = 0.0
	difference += (end.year - start.year) * 12 + end.month - start.month
	start_days = calendar.monthrange(start.year, start.month)[1]
	end_days = calendar.monthrange(end.year, end.month)[1]
	difference += (start_days - start.day) / start_days
	difference += end.day / end_days
	return difference*sign



"""
	Expected task with time differencies:
	count how many period units were elapsed
	from some start time till some end time or now.
"""
def time_division (start, end, period_unit, t_value):
	if t_value > 0 and period_unit in (DAY, WEEK, MONTH, YEAR):
		return {
			DAY: (end - start) / timedelta(days=t_value),
			WEEK: (end - start) / timedelta(weeks=t_value),
			MONTH: diff_month(end, start),
			YEAR: (end - start) / timedelta(days=365.2425*t_value)
			} [period_unit];
	else:
		return 0


class Subject(models.Model):
	extern_id = models.CharField(max_length=256, null=False)
	extern_type = models.IntegerField(null=False)

	def __str__(self):
		return ("%d " % self.extern_type) + self.extern_id

	class Meta:
		verbose_name = "subject (user)"
		verbose_name_plural = "subjects (users)"



class Currency(models.Model):
	name = models.CharField(max_length=100, unique=True, null=False)
	code_num_3 = models.CharField(max_length=3, unique=True, null=False)
	code_ascii_3 = models.CharField(max_length=3, unique=True, null=False)
	exponent = models.IntegerField(null=False)

	def __str__(self):
		return self.code_ascii_3

	def f_val(self, value):
		return num_w_exp(value, self.exponent)

	def s_val(self, value):
		return num_str_exp(value, self.exponent) + " " + self.code_ascii_3

	class Meta:
		verbose_name = "currency"
		verbose_name_plural = "currency"


class Obligation_Period(models.Model):
	subject = models.ForeignKey(Subject, null=False, related_name='obligation_periods')

	comment = models.CharField(max_length=1000, null=True, blank=True)

	# f_value of currancy in t_value of period_unit, for example: $10.00 in 10 days
	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False) # f - financial

	period_unit = models.CharField(max_length=1, choices=PERIOD_UNIT_CHOICES, null=False)
	t_value = models.IntegerField(null=False) # t - time

	start_date = models.DateField(null=False, default=date.today())
	end_date = models.DateField(null=False)

	def __str__(self):
		return self.currency.s_val(self.f_value) + (" / %d" % self.t_value) + " " + self.get_period_unit_display() + " from " + self.start_date.strftime('%Y-%m-%d') + ((" till " + self.end_date.strftime('%Y-%m-%d')) if self.end_date else "")

	class Meta:
		verbose_name = "subject's obligation period"
		verbose_name_plural = "subject's obligation periods"


class Sink(models.Model):
	title = models.CharField(max_length=200, null=False)
	comment = models.CharField(max_length=1000, null=True, blank=True)

	def __str__(self):
		return self.title

	class Meta:
		verbose_name = "payment sink"
		verbose_name_plural = "payment sinks"

#TODO: fixed price sink, periodical sink, fixed price periodical sink

class Sink_Period(models.Model):
	payment_sink = models.ForeignKey(Sink, null=False, related_name='periods')

	comment = models.CharField(max_length=1000, null=True, blank=True)

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	period_unit = models.CharField(max_length=1, choices=PERIOD_UNIT_CHOICES, null=False)
	t_value = models.IntegerField(null=False)
	until_day = models.IntegerField(null=False)
	until_day_condition = models.CharField(max_length=1, choices=UNTIL_CONDITION, null=False)
	# each t_value of period_unit until until_day before|after paying period
	# example: each month until 25-th day before paying period

	start_date = models.DateField(null=False, default=date.today())
	end_date = models.DateField(null=False)

	def __str__ (self):
		if self.t_value != 0:
			return self.currency.s_val(self.f_value) + (" / %d" % self.t_value) + " " + self.get_period_unit_display() + " from " + self.start_date.strftime('%Y-%m-%d') + ((" till " + self.end_date.strftime('%Y-%m-%d')) if self.end_date else "")
		else:
			return self.currency.s_val(self.f_value) + " till " + self.end_date.strftime('%Y-%m-%d')

	class Meta:
		verbose_name = "payment sink period"
		verbose_name_plural = "payment sink periods"


class Event(models.Model):
	commit_timestamp = models.DateTimeField(blank=True, null=False, default=datetime.now())

	event_timestamp = models.DateField(null=False, default=date.today())

	event_type = models.CharField(max_length=1, choices=EVENT_TYPE_CHOICES, null=False)

	comment = models.CharField(max_length=1000, null=True, blank=True)

	parent = models.ForeignKey('self', null=True, related_name='children', blank=True)

	src = models.ForeignKey(Subject, null=True, related_name='src_events', blank=True)
	dst = models.ForeignKey(Sink, null=True, related_name='dst_events', blank=True)

	def __str__(self):
		return self.get_event_type_display() + " " + self.event_timestamp.strftime('%Y-%m-%d') + {
			COMPLEX : "",
			INCOME : " " + str(self.src),
			PAYMENT : " " + str(self.dst),
			SLICE : "",
			MERGE : "",
			EXCHANGE : " " + str(self.exchange_event_data)
			}[self.event_type]

	class Meta:
		verbose_name = "event"
		verbose_name_plural = "events"

# TODO: different tables for different event types - Event - Abstract base class

class Exchange_Event_Data(models.Model):
	src_currency = models.ForeignKey(Currency, null=False, related_name='exchange_src_event_data_set')
	dst_currency = models.ForeignKey(Currency, null=False, related_name='exchange_dst_event_data_set')
	exch_rate = models.IntegerField(null=False)
	exch_rate_exponent = models.IntegerField(null=False)
	comment = models.CharField(max_length=1000, null=True, blank=True)

	event = models.ForeignKey(Event, null=True, related_name='exchange_event_data', blank=True, unique=True)

	def __str__(self):
		return str(self.src_currency) + "/" + str(self.dst_currency) + " " + num_str_exp(self.exch_rate, self.exch_rate_exponent)

	class Meta:
		verbose_name = "exchange event data"
		verbose_name_plural = "exchange event data"




class Money_Chunk(models.Model):
	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	src_event = models.ForeignKey(Event, null=False, related_name='created_money_chunks')
	# if dst_event==None then money chunk is alive
	dst_event = models.ForeignKey(Event, null=True, related_name='deleted_money_chunks', blank=True)

	def __str__(self):
		return self.currency.s_val(self.f_value) + ("alive" if dst_event == None else "")

	class Meta:
		verbose_name = "money chunk"
		verbose_name_plural = "money chunks"






