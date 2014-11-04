from django.db import models
from datetime import date
from dateutil.relativedelta import relativedelta


def num_w_exp (value, exponent):
	return value / (10**exponent)

def num_str_exp (value, exponent):
	return ("%."+("%d" % exponent)+"f") % num_w_exp(value, exponent)



class Subject(models.Model):
	extern_id = models.CharField(max_length=256, null=False)
	extern_type = models.IntegerField(null=False)

	def __unicode__(self):
		return ("%d " % self.extern_type) + self.extern_id

	class Meta:
		verbose_name = "subject (user)"
		verbose_name_plural = "subjects (users)"



class Currency(models.Model):
	name = models.CharField(max_length=100, unique=True, null=False)
	code_num_3 = models.CharField(max_length=3, unique=True, null=False)
	code_ascii_3 = models.CharField(max_length=3, unique=True, null=False)
	exponent = models.IntegerField(null=False)

	def __unicode__(self):
		return self.code_ascii_3

	def f_val(self, value):
		return num_w_exp(value, self.exponent)

	def s_val(self, value):
		return num_str_exp(value, self.exponent) + " " + self.code_ascii_3

	class Meta:
		verbose_name = "currency"
		verbose_name_plural = "currency"


class Period_Unit(models.Model):
	name = models.CharField(max_length=100, unique=True, null=False)

	def __unicode__(self):
		return self.name

	class Meta:
		verbose_name = "period unit"
		verbose_name_plural = "period units"

"""
	В какую сторону округлять?
	Может закодировать это как-то без использования таблицы?
"""
	def delta2units(self, datetime_from, datetime_to):
		return {
			"day" : delta / timedelta(days=1)
			"week" : delta / timedelta(days=7)
			"month" : delta.month()
			"year" : delta.years()
			}[self.name];



#TODO: Amount: f_value + currency
#TODO: Duration: t_value + period_unit


class Obligation_Period(models.Model):
	subject = models.ForeignKey(Subject, null=False, related_name='obligation_periods')

	comment = models.CharField(max_length=1000, null=True, blank=True)

	# f_value of currancy in t_value of period_unit, for example: $10.00 in 10 days
	currency = models.ForeignKey(Currency, null=False, blank=True)
	f_value = models.IntegerField(null=False) # f - financial

	period_unit = models.ForeignKey(Period_Unit, null=False)
	t_value = models.IntegerField(null=False) # t - time

	start_date = models.DateField(null=False)
	end_date = models.DateField(null=True, blank=True)

	def __unicode__(self):
		return self.currency.s_val(self.f_value) + (" / %d" % self.t_value) + " " + str(self.period_unit) + " from " + self.start_date.strftime('%Y-%m-%d') + ((" till " + self.end_date.strftime('%Y-%m-%d')) if self.end_date else "")

	class Meta:
		verbose_name = "subject's obligation period"
		verbose_name_plural = "subject's obligation periods"


class Sink(models.Model):
	title = models.CharField(max_length=200, null=False)
	comment = models.CharField(max_length=1000, null=True, blank=True)

	def __unicode__(self):
		return self.title

	class Meta:
		verbose_name = "payment sink"
		verbose_name_plural = "payment sinks"


class Sink_Period(models.Model):
	payment_sink = models.ForeignKey(Sink, null=False, related_name='periods')

	comment = models.CharField(max_length=1000, null=True, blank=True)

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	period_unit = models.ForeignKey(Period_Unit, null=True, blank=True)
	t_value = models.IntegerField(null=False)

	start_date = models.DateField(null=False)
	end_date = models.DateField(null=True, blank=True)

	def __unicode__ (self):
		if self.t_value != 0:
			return self.currency.s_val(self.f_value) + (" / %d" % self.t_value) + " " + str(self.period_unit) + " from " + self.start_date.strftime('%Y-%m-%d') + ((" till " + self.end_date.strftime('%Y-%m-%d')) if self.end_date else "")
		else:
			return self.currency.s_val(self.f_value) + " till " + self.end_date.strftime('%Y-%m-%d')

	class Meta:
		verbose_name = "payment sink period"
		verbose_name_plural = "payment sink periods"


class Exchange_Event_Data(models.Model):
	src_currency = models.ForeignKey(Currency, null=False, related_name='exchange_src_event_data_set')
	dst_currency = models.ForeignKey(Currency, null=False, related_name='exchange_dst_event_data_set')
	exch_rate = models.IntegerField(null=False)
	exch_rate_exponent = models.IntegerField(null=False)
	comment = models.CharField(max_length=1000, null=True, blank=True)

	def __unicode__(self):
		return self.src_currency + "/" + self.dst_currency + " " + num_str_exp(self.exch_rate, self.exch_rate_exponent)

	class Meta:
		verbose_name = "exchange event data"
		verbose_name_plural = "exchange event data"


class Event(models.Model):
	commit_timestamp = models.DateTimeField(null=False)
	event_timestamp = models.DateField(null=False)

	# 0 - complex event (sequence of events) - several events has it's id as parent
	# 1 - income (subject deposit) - src not Null
	# 2 - payment - dst not Null
	# 3 - slice - utility event, delete one and create 2 or more chunks
	# 4 - merge - utility event, delete 2 or more and create 1 chunk
	# 5 - exchange - exchange_event_data not Null
	type_id = models.IntegerField(null=False)

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
		return type_str(self.type_id)

	comment = models.CharField(max_length=1000, null=True, blank=True)

	parent = models.ForeignKey('self', null=True, related_name='children', blank=True)

	src = models.ForeignKey(Subject, null=True, related_name='src_events', blank=True)
	dst = models.ForeignKey(Sink, null=True, related_name='dst_events', blank=True)
	exchange_event_data = models.ForeignKey(Exchange_Event_Data, null=True, related_name='exch_events', blank=True)

	def __unicode__(self):
		return self.type_name() + " " + self.event_timestamp + {
			0 : "",
			1 : " " + self.src,
			2 : " " + self.dst,
			3 : "",
			4 : "",
			5 : " " + self.exchange_event_data
			}[self.type_id]

	class Meta:
		verbose_name = "event"
		verbose_name_plural = "events"


class Money_Chunk(models.Model):
	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	src_event = models.ForeignKey(Event, null=False, related_name='created_money_chunks')
	# if dst_event==None then money chunk is alive
	dst_event = models.ForeignKey(Event, null=True, related_name='deleted_money_chunks', blank=True)

	def __unicode__(self):
		return self.currency.s_val(self.f_value) + ("alive" if dst_event == None else "")

	class Meta:
		verbose_name = "money chunk"
		verbose_name_plural = "money chunks"






