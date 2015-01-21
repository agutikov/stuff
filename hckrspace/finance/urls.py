from django.conf.urls import url

from finance import views

urlpatterns = [
    url(r'^$', views.index, name='index'),
    url(r'^transactions/$', views.TransactionList.as_view(), name='transactions'),
]