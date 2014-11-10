from django.contrib import admin
from finance.models import Subject, Currency, Obligation_Period, Sink, Sink_Period, Exchange_Event_Data, Event, Money_Chunk


admin.site.register(Currency)

admin.site.register(Subject)
admin.site.register(Obligation_Period)

admin.site.register(Sink)
admin.site.register(Sink_Period)

"""
class Event_Inline(admin.StackedInline):
	model = Event

class Exchange_Event_Data_Admin(admin.ModelAdmin):
	inlines = [Event_Inline]

admin.site.register(Exchange_Event_Data, Exchange_Event_Data_Admin)
"""

admin.site.register(Exchange_Event_Data)


class Exchange_Event_Data_Inline(admin.StackedInline):
	model = Exchange_Event_Data

"""
class Money_Chunk_Src_Inline(admin.StackedInline):
	model = Money_Chunk
	extra = 1

class Money_Chunk_Dst_Inline(admin.StackedInline):
	model = Money_Chunk
	extra = 1
"""

class EventAdmin(admin.ModelAdmin):
	fieldsets = [
		(None,               {'fields': ['event_timestamp', 'event_type', 'comment']}),
		('Part of composite event', {'fields': ['parent'], 'classes': ['collapse']}),
		('Income', {'fields': ['src']}),
		('Payment', {'fields': ['dst']}),
	]
	inlines = [Exchange_Event_Data_Inline]

admin.site.register(Event, EventAdmin)

admin.site.register(Money_Chunk)


