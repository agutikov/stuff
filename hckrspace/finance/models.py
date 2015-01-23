from django.db import models
from datetime import datetime, date, timedelta
from dateutil.relativedelta import relativedelta
import calendar
from django.contrib.humanize.templatetags.humanize import intcomma
from django.db.models.signals import post_save
from django.db.models import Max


def num_w_exp (value, exponent):
	return value / (10**exponent)

def num_str_exp (value, exponent):
	return '%s%s' % (intcomma(int(num_w_exp(value, exponent))),
		  (("%."+("%d" % exponent)+"f") % num_w_exp(value, exponent))[-(exponent+1):]
		  if exponent > 0 else '')

# Валюта
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


# Субъект с внешним (относительно этого модуля) id который может делать взносы
class Subject(models.Model):
	extern_id = models.CharField(max_length=256, null=False)
	extern_model_name = models.CharField(max_length=256, null=False)
	extern_app_name = models.CharField(max_length=256, null=False)

	def __str__(self):
		return ("%s.%s.%s" % (self.extern_app_name, self.extern_model_name, self.extern_id))

	class Meta:
		verbose_name = "subject (user)"
		verbose_name_plural = "subjects (users)"


# Назначение платежей
class Contract(models.Model):
	title = models.CharField(max_length=200, null=False)
	details = models.TextField(null=True, blank=True)

	currency = models.ForeignKey(Currency, null=True, blank=True)
	f_value = models.IntegerField(null=True, blank=True)

	def __str__(self):
		return self.title

	class Meta:
		verbose_name = "contract"
		verbose_name_plural = "contracts"


# Счёт
class Bill(models.Model):
	destination = models.ForeignKey(Contract, null=False, related_name='bills')

	details = models.TextField(null=True, blank=True)

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	# Оплачиваемый период (если актуально)
	from_date = models.DateField(null=True, blank=True)
	till_date = models.DateField(null=True, blank=True)

	# Оплатить до
	due_date = models.DateField(null=False)

	def __str__(self):
		return str(self.destination) + ' ' + self.currency.s_val(self.f_value)

	class Meta:
		verbose_name = "bill"
		verbose_name_plural = "bills"


# Информация хранится в ивентах, привязанных к транзакции
# Законченная операция (взнос, оплата, обмен) которая может включать в себя несколько событий изменения баланса
class Transaction(models.Model):
	timestamp = models.DateTimeField(blank=True, null=False, default=datetime.now())

	description = models.TextField(null=True, blank=True)

	def __str__(self):
		view = self.timestamp.strftime('%Y-%m-%d:')
		if (self.events):
			for event in self.events.all():
				view += ' ' + event.currency.s_val(event.f_value) + ';'
		else:
			view += ' ' + self.description

		return view

	class Meta:
		verbose_name = "transaction"
		verbose_name_plural = "transactions"


# Изменение баланса на определённую положительную или отрицательную сумму
class Event(models.Model):
	transaction = models.ForeignKey(Transaction, null=False, related_name='events')

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	def __str__(self):
		view = self.transaction.timestamp.strftime('%Y-%m-%d ')

		view += self.currency.s_val(self.f_value)

		return view

	class Meta:
		verbose_name = "event"
		verbose_name_plural = "events"


# add new totals and balance after save new event
def after_save_event(sender, instance, **kwargs):
	# add new balance
	last_balance = Balance.objects.filter(currency=instance.currency).latest('transaction__timestamp')
	new_balance = Balance(transaction=instance.transaction, currency=instance.currency, f_value=last_balance.f_value + instance.f_value)
	new_balance.save()
	# add new total
	last_total = Total.objects.filter(currency=instance.currency, direction=(instance.f_value > 0)).latest('transaction__timestamp')
	new_total = Total(transaction=instance.transaction, currency=instance.currency, f_value=last_total.f_value + instance.f_value, direction=(instance.f_value > 0))
	new_total.save()

# register the post_save signal
post_save.connect(after_save_event, sender=Event, dispatch_uid=__file__)


# Оплата одного счёта
class Payment(models.Model):
	bill = models.ForeignKey(Bill, unique=True, null=False, related_name='payment')

	transaction = models.ForeignKey(Transaction, unique=True, null=False, related_name='payment')

	comment = models.TextField(null=True, blank=True)

	def __str__(self):
		return str(self.bill) + ' ' + str(self.transaction)

	class Meta:
		verbose_name = "payment"
		verbose_name_plural = "payments"


# Членский взнос
class Membership_fee(models.Model):
	subject = models.ForeignKey(Subject, null=False, related_name='membership_fees')

	transaction = models.ForeignKey(Transaction, unique=True, null=False, related_name='membership_fee')

	comment = models.TextField(null=True, blank=True)

	def __str__(self):
		return str(self.subject) + ' ' + str(self.transaction)

	class Meta:
		verbose_name = "membership_fee"
		verbose_name_plural = "membership_fees"


# Сумма взносов и оплат по каждой валюте (положительная и отрицательная)
# Новая запись добавляется в after_save_event
class Total(models.Model):
	transaction = models.ForeignKey(Transaction, null=False, related_name='totals')

	# income - true, consumption - false
	direction = models.BooleanField(default=None)

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	def __str__(self):
		return str(self.transaction.timestamp) + ' ' + self.currency.s_val(self.f_value)

	class Meta:
		verbose_name = "total"
		verbose_name_plural = "totals"


# Состояние баланса по каждой валюте
# Новая запись добавляется в after_save_event
class Balance(models.Model):
	transaction = models.ForeignKey(Transaction, null=False, related_name='balances')

	currency = models.ForeignKey(Currency, null=False)
	f_value = models.IntegerField(null=False)

	def __str__(self):
		return str(self.transaction.timestamp) + ' ' + self.currency.s_val(self.f_value)

	class Meta:
		verbose_name = "balance"
		verbose_name_plural = "balances"








