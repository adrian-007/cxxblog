/*
 * Copyright (C) 2020 adrian_007, adrian-007 on o2 point pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#pragma once

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/StdSqlTraits.h>

#include <Wt/WDate.h>
#include <Wt/WString.h>

namespace dbo = Wt::Dbo;

class Editor;

class EditorJobOffer
    : public dbo::Dbo<EditorJobOffer>
{
public:
    enum class Status
    {
        New,
        Expired,
        Archived,
        Applied,
        Rejected,
        InterviewFailed,
        NegotiationsFailed,
        PendingApproval,
        Hired,
        Left,
        Fired
    };

    enum class EmploymentType
    {
        Consultant,
        EmploymentAgreement
    };

    dbo::ptr<Editor> editor;

    Wt::WString company;
    Wt::WString companyUrl;

    Wt::WString position;
    Wt::WString description;

    Wt::WString currency;
    int monthlyRate;
    int numberOfPaidTimeOffDaysPerYear; // Applies to contracts only.
    int workTimePercentage;

    Wt::WDate created;
    Wt::WDate applicationDate;
    Wt::WDate resolutionDate;

    Status status;
    Wt::WString notes;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, editor, "editor", dbo::NotNull | dbo::OnUpdateCascade | dbo::OnDeleteCascade);

        dbo::field(a, company, "company");
        dbo::field(a, companyUrl, "companyUrl");
        dbo::field(a, position, "position");
        dbo::field(a, description, "description");
        dbo::field(a, currency, "currency");
        dbo::field(a, monthlyRate, "monthly_rate");
        dbo::field(a, numberOfPaidTimeOffDaysPerYear, "number_of_paid_time_off_days_per_year");
        dbo::field(a, workTimePercentage, "work_time_percentage");
        dbo::field(a, created, "created");
        dbo::field(a, applicationDate, "application_date");
        dbo::field(a, resolutionDate, "resolution_date");
        dbo::field(a, status, "status");
        dbo::field(a, notes, "notes");
    }
};
