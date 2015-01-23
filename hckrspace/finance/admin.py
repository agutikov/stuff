from django.contrib import admin
from django import forms
from django.db import models
from finance.models import Subject, Currency, Contract, Bill, Transaction, Event, Payment, Membership_fee, Total, Balance


class Currency_Admin(admin.ModelAdmin):
	list_display = ('id', 'name', 'code_num_3', 'code_ascii_3', 'exponent')
	list_display_links = ['name']

admin.site.register(Currency, Currency_Admin)

admin.site.register(Subject)



class Bill_Admin(admin.ModelAdmin):
	list_display = ('id', 'destination', 'details', 'currency', 'f_value', 'from_date', 'till_date', 'due_date')
	list_display_links = ['id']
	list_editable = ('destination', 'currency', 'f_value', 'from_date', 'till_date', 'due_date')
	search_fields = ['details']
	list_filter = ['destination']


admin.site.register(Bill, Bill_Admin)


class Bill_Inline(admin.TabularInline):
	verbose_name = 'Bill'
	verbose_name_plural = 'Bills'
	model = Bill
	fk_name = 'destination'
	extra = 1
	can_delete = True
	readonly_fields = ['details']

class Contract_Admin(admin.ModelAdmin):
	inlines = [Bill_Inline]

	list_display = ('id', 'title', 'details', 'currency', 'f_value')
	list_display_links = ['id']
	list_editable = ('title', 'currency', 'f_value')
	search_fields = ['title', 'details']

admin.site.register(Contract, Contract_Admin)



class Event_Admin(admin.ModelAdmin):
	list_display = ('id', 'transaction', 'currency', 'f_value')
	list_display_links = ['id']
	list_editable = ('transaction', 'currency', 'f_value')
#	search_fields = ['details']
#	list_filter = ['destination']


admin.site.register(Event, Event_Admin)


class Event_Inline(admin.TabularInline):
	verbose_name = 'Event'
	verbose_name_plural = 'Events'
	model = Event
	fk_name = 'transaction'
	extra = 1
	can_delete = True

class Transaction_Admin(admin.ModelAdmin):
	inlines = [Event_Inline]

	list_display = ('id', 'timestamp', 'description')
	list_display_links = ['id']
	list_editable = []
	search_fields = ['description']


admin.site.register(Transaction, Transaction_Admin)





admin.site.register(Payment)
admin.site.register(Membership_fee)
admin.site.register(Total)
admin.site.register(Balance)








