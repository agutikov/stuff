from django.db import models
from datetime import datetime, date, timedelta
from dateutil.relativedelta import relativedelta
import calendar
from django.contrib.humanize.templatetags.humanize import intcomma

def num_w_exp (value, exponent):
	return value / (10**exponent)

def num_str_exp (value, exponent):
	return '%s%s' % (intcomma(int(num_w_exp(value, exponent))),
		  (("%."+("%d" % exponent)+"f") % num_w_exp(value, exponent))[-(exponent+1):]
		  if exponent > 0 else '')



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


class Subject(models.Model):
	extern_id = models.CharField(max_length=256, null=False)
	extern_type = models.IntegerField(null=False)

	def __str__(self):
		return ("%d " % self.extern_type) + self.extern_id

	class Meta:
		verbose_name = "subject (user)"
		verbose_name_plural = "subjects (users)"



class Payment_Destination(models.Model):
	title = models.CharField(max_length=200, null=False)
	comment = models.TextField(null=True, blank=True)

	currency = models.ForeignKey(Currency, null=True, blank=True)
	f_value = models.IntegerField(null=True, blank=True)

	def __str__(self):
		return self.title

	class Meta:
		verbose_name = "payment destination"
		verbose_name_plural = "payment destination"


class Payment_Obligation(models.Model):
	destination = models.ForeignKey(Payment_Destination, null=False, related_name='payment_obligations')

	title = models.CharField(max_length=200, null=False)
	comment = models.TextField(null=True, blank=True)

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	from_date = models.DateField(null=True, blank=True)
	till_date = models.DateField(null=True, blank=True)

	payment_date = models.DateField(null=False)

	def __str__(self):
		return self.title + ' ' + self.currency.s_val(self.f_value)

	class Meta:
		verbose_name = "payment obligation"
		verbose_name_plural = "payment obligation"



class Event(models.Model):
	commit_timestamp = models.DateTimeField(blank=True, null=False, default=datetime.now())

	event_timestamp = models.DateField(null=False, default=date.today())

	comment = models.TextField(null=True, blank=True)

	src = models.ForeignKey(Subject, related_name='src_events', null=True, blank=True)

	dst = models.ForeignKey(Payment_Obligation, related_name='dst_events', unique=True, null=True, blank=True)

	src_currency = models.ForeignKey(Currency, related_name='exchange_src_event', null=True, blank=True)
	dst_currency = models.ForeignKey(Currency, related_name='exchange_dst_event', null=True, blank=True)
	exch_rate = models.IntegerField(null=True, blank=True)
	exch_rate_exponent = models.IntegerField(null=True, blank=True)

	def __str__(self):
		view = self.event_timestamp.strftime('%Y-%m-%d')
		if self.src:
			view += ' ' + str(self.src)
		if self.dst:
			view += ' ' + str(self.dst)

		if self.src_currency and self.dst_currency and self.exch_rate and self.exch_rate_exponent:
			view += ' ' + str(self.dst_currency) + '/' + str(self.src_currency) + ' ' + num_str_exp(self.exch_rate, self.exch_rate_exponent)

		return view

	class Meta:
		verbose_name = "event"
		verbose_name_plural = "events"



class Money_Chunk(models.Model):
	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	src_event = models.ForeignKey(Event, related_name='created_money_chunks', null=False)
	dst_event = models.ForeignKey(Event, related_name='deleted_money_chunks', null=True, blank=True)

	def __str__(self):
		if self.dst_event:
			return self.currency.s_val(0)
		else:
			return self.currency.s_val(self.f_value)

	class Meta:
		verbose_name = "money chunk"
		verbose_name_plural = "money chunks"






