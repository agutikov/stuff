from django.contrib import admin
from finance.models import Subject, Currency, Obligation_Period, Sink, Sink_Period, Exchange_Event_Data, Event, Money_Chunk


admin.site.register(Currency)

admin.site.register(Subject)
admin.site.register(Obligation_Period)

admin.site.register(Sink)
admin.site.register(Sink_Period)

admin.site.register(Exchange_Event_Data)
admin.site.register(Event)

admin.site.register(Money_Chunk)


