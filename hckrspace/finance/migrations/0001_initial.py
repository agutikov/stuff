# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import datetime


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Balance',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('f_value', models.IntegerField()),
            ],
            options={
                'verbose_name': 'balance',
                'verbose_name_plural': 'balances',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Bill',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('details', models.TextField(blank=True, null=True)),
                ('f_value', models.IntegerField()),
                ('from_date', models.DateField(blank=True, null=True)),
                ('till_date', models.DateField(blank=True, null=True)),
                ('due_date', models.DateField()),
            ],
            options={
                'verbose_name': 'bill',
                'verbose_name_plural': 'bills',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Contract',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('title', models.CharField(max_length=200)),
                ('details', models.TextField(blank=True, null=True)),
                ('f_value', models.IntegerField(blank=True, null=True)),
            ],
            options={
                'verbose_name': 'contract',
                'verbose_name_plural': 'contracts',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Currency',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('name', models.CharField(unique=True, max_length=100)),
                ('code_num_3', models.CharField(unique=True, max_length=3)),
                ('code_ascii_3', models.CharField(unique=True, max_length=3)),
                ('exponent', models.IntegerField()),
            ],
            options={
                'verbose_name': 'currency',
                'verbose_name_plural': 'currency',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Event',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('f_value', models.IntegerField()),
                ('currency', models.ForeignKey(to='finance.Currency')),
            ],
            options={
                'verbose_name': 'event',
                'verbose_name_plural': 'events',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Membership_fee',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('comment', models.TextField(blank=True, null=True)),
            ],
            options={
                'verbose_name': 'membership_fee',
                'verbose_name_plural': 'membership_fees',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Payment',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('comment', models.TextField(blank=True, null=True)),
                ('bill', models.ForeignKey(unique=True, related_name='payment', to='finance.Bill')),
            ],
            options={
                'verbose_name': 'payment',
                'verbose_name_plural': 'payments',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Subject',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('extern_id', models.CharField(max_length=256)),
                ('extern_model_name', models.CharField(max_length=256)),
                ('extern_app_name', models.CharField(max_length=256)),
            ],
            options={
                'verbose_name': 'subject (user)',
                'verbose_name_plural': 'subjects (users)',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Total',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('direction', models.BooleanField(default=None)),
                ('f_value', models.IntegerField()),
                ('currency', models.ForeignKey(to='finance.Currency')),
            ],
            options={
                'verbose_name': 'total',
                'verbose_name_plural': 'totals',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Transaction',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('timestamp', models.DateTimeField(default=datetime.datetime(2015, 1, 23, 16, 16, 53, 113062), blank=True)),
                ('description', models.TextField(blank=True, null=True)),
            ],
            options={
                'verbose_name': 'transaction',
                'verbose_name_plural': 'transactions',
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='total',
            name='transaction',
            field=models.ForeignKey(related_name='totals', to='finance.Transaction'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='payment',
            name='transaction',
            field=models.ForeignKey(unique=True, related_name='payment', to='finance.Transaction'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='membership_fee',
            name='subject',
            field=models.ForeignKey(related_name='membership_fees', to='finance.Subject'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='membership_fee',
            name='transaction',
            field=models.ForeignKey(unique=True, related_name='membership_fee', to='finance.Transaction'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='event',
            name='transaction',
            field=models.ForeignKey(related_name='events', to='finance.Transaction'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='contract',
            name='currency',
            field=models.ForeignKey(null=True, blank=True, to='finance.Currency'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='bill',
            name='currency',
            field=models.ForeignKey(to='finance.Currency'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='bill',
            name='destination',
            field=models.ForeignKey(related_name='bills', to='finance.Contract'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='balance',
            name='currency',
            field=models.ForeignKey(to='finance.Currency'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='balance',
            name='transaction',
            field=models.ForeignKey(related_name='balances', to='finance.Transaction'),
            preserve_default=True,
        ),
    ]
