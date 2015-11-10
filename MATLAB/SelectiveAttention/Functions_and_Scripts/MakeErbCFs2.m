function cfs = MakeErbCFs2(mincf,maxcf,numchans)

cfs = ErbRateToHz(linspace(HzToErbRate(mincf),HzToErbRate(maxcf),numchans));

end

function y=ErbRateToHz(x)

y=(10.^(x/21.4)-1)/4.37e-3;

end



function y=HzToErbRate(x)


y=(21.4*log10(4.37e-3*x+1));


end
