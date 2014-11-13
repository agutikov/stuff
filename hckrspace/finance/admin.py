from django.contrib import admin
from finance.models import Subject, Currency, Obligation_Period, Sink, Sink_Period, Exchange_Event_Data, Event, Money_Chunk


admin.site.register(Currency)

admin.site.register(Subject)
admin.site.register(Obligation_Period)

admin.site.register(Sink)
admin.site.register(Sink_Period)

admin.site.register(Exchange_Event_Data)


class Created_Money_Chunk_Inline(admin.StackedInline):
	verbose_name = 'Created money chunk'
	verbose_name_plural = 'Created money chunks'
	model = Money_Chunk
	fk_name = 'src_event'
	extra = 1
	fieldsets = [
		(None, {'fields' : ['currency', 'f_value']}),
		('Dst event', {'fields': ['dst_event'], 'classes': ['collapse']})
		]

class Deleted_Money_Chunk_Inline(admin.StackedInline):
	verbose_name = 'Deleted money chunk'
	verbose_name_plural = 'Deleted money chunks'
	model = Money_Chunk
	fk_name = 'dst_event'
	extra = 1
	fieldsets = [
		(None, {'fields' : ['currency', 'f_value']}),
		('Src event', {'fields': ['src_event']})
		]
'''
class Exchange_Event_Data_Inline(admin.StackedInline):
	model = Exchange_Event_Data
'''
class Event_Admin(admin.ModelAdmin):
	fieldsets = [
		(None,               {'fields': ['event_timestamp', 'event_type', 'comment']}),
		('Part of composite event', {'fields': ['parent'], 'classes': ['collapse']}),
		('Income', {'fields': ['src']}),
		('Payment', {'fields': ['dst']}),
	]
	inlines = [Created_Money_Chunk_Inline, Deleted_Money_Chunk_Inline]

admin.site.register(Event, Event_Admin)

'''
class Creation_Event_Inline(admin.StackedInline):
	verbose_name = 'Creation Event'
	model = Event

class Deletion_Event_Inline(admin.StackedInline):
	verbose_name = 'Deletion Event'
	model = Event


class Money_Chunk_Admin(admin.ModelAdmin):
	inlines = [Creation_Event_Inline, Deletion_Event_Inline]


admin.site.register(Money_Chunk, Money_Chunk_Admin)

'''

admin.site.register(Money_Chunk)
