from django.contrib import admin
from django import forms
from django.db import models
from finance.models import Subject, Currency, Payment_Destination, Payment_Obligation, Event, Money_Chunk


class Currency_Admin(admin.ModelAdmin):
	list_display = ('id', 'name', 'code_num_3', 'code_ascii_3', 'exponent')
	list_display_links = ['name']

admin.site.register(Currency, Currency_Admin)


class Money_Chunk_Admin(admin.ModelAdmin):
	list_display = ('id', 'currency', 'f_value', 'src_event', 'dst_event')
	list_display_links = ['id']
	list_editable = ('currency', 'f_value', 'src_event', 'dst_event')
	list_filter = ['currency']

admin.site.register(Money_Chunk, Money_Chunk_Admin)


class Created_Money_Chunk_Inline(admin.TabularInline):
	verbose_name = 'Created money chunk'
	verbose_name_plural = 'Created money chunks'
	model = Money_Chunk
	fk_name = 'src_event'
	extra = 1
	can_delete = True
	fieldsets = [
		(None, {'fields' : ['currency', 'f_value']}),
		('Src event', {'fields': ['src_event']}),
		('Dst event', {'fields': ['dst_event'], 'classes': ['collapse']})
		]

class Deleted_Money_Chunk_Inline(admin.TabularInline):
	verbose_name = 'Deleted money chunk'
	verbose_name_plural = 'Deleted money chunks'
	model = Money_Chunk
	fk_name = 'dst_event'
	extra = 0
	can_delete = True
	fieldsets = [
		(None, {'fields' : ['currency', 'f_value']}),
		('Src event', {'fields': ['src_event']}),
		('Dst event', {'fields': ['dst_event']})
		]
	readonly_fields = ('currency', 'f_value', 'src_event', 'dst_event')

class Event_Admin(admin.ModelAdmin):
	fieldsets = [
		(None,               {'fields': ['event_timestamp', 'comment']}),
		('Income', {'fields': ['src']}),
		('Payment', {'fields': ['dst']}),
		('Exchange', {'fields': ['src_currency', 'dst_currency', 'exch_rate', 'exch_rate_exponent']}),
	]
	inlines = [Created_Money_Chunk_Inline, Deleted_Money_Chunk_Inline]

	list_display = ('id', 'event_timestamp', 'comment', 'src', 'dst', 'src_currency', 'dst_currency', 'exch_rate', 'exch_rate_exponent')
	list_display_links = ['id']
	list_editable = ('event_timestamp', 'src', 'dst', 'src_currency', 'dst_currency', 'exch_rate', 'exch_rate_exponent')
	search_fields = ['comment']
	list_filter = ['src']



admin.site.register(Event, Event_Admin)


admin.site.register(Subject)


class Payment_Obligation_Admin(admin.ModelAdmin):
	list_display = ('id', 'destination', 'title', 'comment', 'currency', 'f_value', 'from_date', 'till_date', 'payment_date')
	list_display_links = ['id']
	list_editable = ('destination', 'title', 'currency', 'f_value', 'from_date', 'till_date', 'payment_date')
	search_fields = ['title', 'comment']
	list_filter = ['destination']


admin.site.register(Payment_Obligation, Payment_Obligation_Admin)


class Payment_Obligation_Inline(admin.TabularInline):
	verbose_name = 'Obligation'
	verbose_name_plural = 'Obligations'
	model = Payment_Obligation
	fk_name = 'destination'
	extra = 1
	can_delete = True

	readonly_fields = ['comment']

class Payment_Destination_Admin(admin.ModelAdmin):
	inlines = [Payment_Obligation_Inline]

	list_display = ('id', 'title', 'comment', 'currency', 'f_value')
	list_display_links = ['id']
	list_editable = ('title', 'currency', 'f_value')
	search_fields = ['title', 'comment']

admin.site.register(Payment_Destination, Payment_Destination_Admin)




