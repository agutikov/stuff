from django.http import HttpResponse

from django.views.generic import ListView

from finance.models import Subject, Currency, Contract, Bill, Transaction, Event, Payment, Membership_fee, Total, Balance

class TransactionList(ListView):
	model = Transaction


def transaction_detail(request, transaction_id):

	tr = Transaction.objects.get(id=transaction_id)

	return HttpResponse(tr)


#import logging
from pprint import pprint

#logger = logging.getLogger('project.interesting.stuff')

def index(request):
	
#	logger.debug(request.META)
	pprint(request.META)
	
	return HttpResponse("Hello, world. You're at the finance index.")
