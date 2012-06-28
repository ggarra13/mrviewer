class RenderTransformsController < ApplicationController
  def index
    list
    render :action => 'list'
  end

  # GETs should be safe (see http://www.w3.org/2001/tag/doc/whenToUseGet.html)
  verify :method => :post, :only => [ :destroy, :create, :update ],
         :redirect_to => { :action => :list }

  def list
    @render_transform_pages, @render_transforms = paginate :render_transforms, :per_page => 10
  end

  def show
    @render_transform = RenderTransform.find(params[:id])
  end

  def new
    @render_transform = RenderTransform.new
  end

  def create
    @render_transform = RenderTransform.new(params[:render_transform])
    if @render_transform.save
      flash[:notice] = 'RenderTransform was successfully created.'
      redirect_to :action => 'list'
    else
      render :action => 'new'
    end
  end

  def edit
    @render_transform = RenderTransform.find(params[:id])
  end

  def update
    @render_transform = RenderTransform.find(params[:id])
    if @render_transform.update_attributes(params[:render_transform])
      flash[:notice] = 'RenderTransform was successfully updated.'
      redirect_to :action => 'show', :id => @render_transform
    else
      render :action => 'edit'
    end
  end

  def destroy
    RenderTransform.find(params[:id]).destroy
    redirect_to :action => 'list'
  end
end
