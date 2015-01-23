from django.conf.urls import url

from finance import views

urlpatterns = [
    url(r'^$', views.index, name='index'),
    url(r'^transactions/$', views.TransactionList.as_view(), name='transactions'),
    # ex: /finance/transactions/5/
    url(r'^transactions/(?P<transaction_id>\d+)/$', views.transaction_detail, name='detail'),
]